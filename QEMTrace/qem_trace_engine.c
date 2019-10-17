/*
 * Guest memory access tracing engine.
 *
 * Provides tools to translate virtual addresses to physical addreses.
 * Provides tools to save traces in trace files.
 *
 *  Copyright (c) 2019 Alexy Torres Aurora Dugo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "qemu/osdep.h"
#include "qemu/host-utils.h"
#include "cpu.h"
#include "qemu/timer.h"

#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

#include "qem_trace_engine.h" /* Engine header */
#include "qem_trace_logger.h" /* QEM logger */

/*******************************************************************************
 * GLOBAL VARS
 ******************************************************************************/
/* Keeps track on the tracing state */
volatile uint32_t qem_tracing_enabled = 0;

/* Keeps track execution time */
volatile uint64_t qem_time_start = 0;

/* Commnuicator connection state */
uint32_t connected = 0;

#if QEM_TRACE_PRINT == 0
/* This is used to implement the write buffer. This buffer contains the last
 * memory traces to be written back to the disk. Using the buffer allows to
 * avoid having to call "write" at avery tracing point and allow a nearly-null
 * overhead when writing to the file.
 */
static uint32_t qem_trace_buffer_size = 0;
static uint64_t qem_file_size = 0;
static uint8_t  qem_trace_buffer[QEM_TRACE_BUFFER_SIZE * sizeof(qem_trace_t)];
static FILE*    qem_trace_file_fd;

/* This is the file index ID. Each time the tracing is started, the index is
 * incremented and the new file name will be of the form qem_trace_xx.out where
 * xx is the file_index.
 */
static uint32_t qem_file_index = 0;
#endif /* #if QEM_TRACE_PRINT == 0 && QEM_TRACE_STREAM  */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
#if QEM_TRACE_PRINT == 0
static void qem_write_header(const uint64_t size, const uint32_t trace_size,
                             const uint32_t trace_version)
{
    int error;

    qem_trace_header_t header = 
    {
            .size = size,
            .struct_size = trace_size,
            .version = trace_version
    };

    /* Go to the begining of the file */
    error = fseek(qem_trace_file_fd, 0, SEEK_SET);
    if(error != 0)
    {
        QEM_TRACE_ERROR("Could not set the header marker", error, 1);
    }

    /* Write the header */
    error = fwrite(&header, sizeof(qem_trace_header_t),
                   1, qem_trace_file_fd);
    if(error != 0)
    {
        QEM_TRACE_ERROR("Could write the header", error, 1);
    }

    /* Reset the buffer size */
    qem_trace_buffer_size = 0;
}

static void qem_init_tracing(void)
{
    /* Open file descriptor, the non block option must be verified but this may
     * allow us to gain a lot of time
     */
    char filename[23];
    sprintf(filename, "qem_trace_%08d.out", file_index++);
    filename[22] = 0;
    remove(filename);

    qem_trace_file_fd = fopen(filename, "wb+");

    /* In case of error */
    if(qem_trace_file_fd == NULL)
    {
        QEM_TRACE_ERROR("Could not create or open the trace file", errno, 1);
    }

    /* Init the header space */
    write_header(0, sizeof(qem_trace_t), 1);
    file_size = sizeof(qem_trace_t);

    QEM_TRACE_INFO("==== Trace file name", 0);
    QEM_TRACE_INFO(filename);
}

static void qem_close_tracing(void)
{
    int error;

    /* Check buffer state and flush */
    if(qem_trace_buffer_size != 0)
    {
        error = fwrite(qem_trace_buffer, sizeof(qem_trace_t),
                       qem_trace_buffer_size, qem_trace_file_fd);

        if(error != 0)
        {
            QEM_TRACE_ERROR("Could flush trace buffer", error, 1);
        }

        file_size += qem_trace_buffer_size * sizeof(qem_trace_t);

        qem_trace_buffer_size = 0;
    }

    write_header(file_size, sizeof(qem_trace_t), 1);

    /* Close file */
    if(fclose(qem_trace_file_fd) < 0)
    {
        QEM_TRACE_ERROR("Error while closing the trace output file", errno, 0);
    }
    else
    {
        QEM_TRACE_INFO("Trace file saved", 0);
    }
}
#endif /* #if QEM_TRACE_PRINT == 0 */

void qem_trace_enable(CPUState* cs)
{
    if(qem_tracing_enabled == 1)
    {
        return;
    }

    /* Enable tracing state */
    qem_tracing_enabled = 1;

#if QEM_TRACE_PRINT == 0
    /* Open output file */
    qem_init_tracing();
#endif

#ifdef TARGET_I386
    /* Flush software tlb */
    CPUArchState *env = cs->env_ptr;

    /* The QOM tests will trigger tlb_flushes without setting up TCG
     * so we bug out here in that case.
     */
    if (!tcg_enabled()) 
    {
        QEM_TRACE_ERROR("Error while enabling traces", QEM_ERROR_TCG_DIS, 0);
    }

    atomic_set(&env->tlb_flush_count, env->tlb_flush_count + 1);

    memset(env->tlb_table, -1, sizeof(env->tlb_table));
    memset(env->tlb_v_table, -1, sizeof(env->tlb_v_table));
    cpu_tb_jmp_cache_clear(cs);

    env->vtlb_index = 0;
    env->tlb_flush_addr = -1;
    env->tlb_flush_mask = 0;

    atomic_mb_set(&cs->pending_tlb_flush, 0);
#endif
}

void qem_trace_disable(void)
{
    if(qem_tracing_enabled == 0)
    {
        return;
    }

    /* Enable tracing state */
    qem_tracing_enabled = 0;

#if QEM_TRACE_PRINT == 0
    /* Close output file or stream */
    qem_close_tracing();
#endif
}

void qem_trace_start_time(void)
{
    /* Save the current time */
    qem_time_start = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);
}

void qem_trace_output_time(void)
{
    /* Get the current time and compares it to the previously defined start
     * time.
     */
    uint64_t time_end;
    time_end = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);
    printf("[QEMU] Elapsed time = %" PRIu64 "\n", (time_end - qem_time_start));
}

#if QEM_TRACE_TARGET_64
void qem_trace_output(uint64_t virt_addr, uint64_t phys_addr,
                      uint32_t core, uint64_t time, uint32_t flags)
#else
void qem_trace_output(uint32_t virt_addr, uint32_t phys_addr,
                      uint32_t core, uint64_t time, uint32_t flags)
#endif
{
#if QEM_TRACE_PRINT == 0
    int error;

    (void)virt_addr;
    /* Note that these functions are called at each instruction, this is why we
     * do not perform any test on the file descriptor nor the write execution
     * state.
     */

    /* The buffer size is a multiple of the trace size so we just check if we
     * can add a new entry */
     if(qem_trace_buffer_size == QEM_TRACE_BUFFER_SIZE)
     {
        error = fwrite(qem_trace_buffer, sizeof(qem_trace_t),
                       QEM_TRACE_BUFFER_SIZE, qem_trace_file_fd);

        if(error != 0)
        {
            QEM_TRACE_ERROR("Could flush trace buffer", error, 1);
        }

        file_size += QEM_TRACE_BUFFER_SIZE * sizeof(qem_trace_t);

        qem_trace_buffer_size = 0;
     }

     /* Create the structure */
     qem_trace_t new_trace = {
         .address = phys_addr,
         .timestamp = time,
         .flags = (core & 0xFF) | (flags & 0xFFFFFF00)
     };

     /* Copy the structure to the buffer */
     uint64_t offset = qem_trace_buffer_size * sizeof(qem_trace_t);
     memcpy(qem_trace_buffer + offset, &new_trace, sizeof(qem_trace_t));
     ++qem_trace_buffer_size;
#else

     ((flags & QEM_TRACE_EVENT_DCBZ) == QEM_TRACE_EVENT_DCBZ) ? printf("D ") : (
         ((flags & QEM_TRACE_EVENT_PREFETCH) == QEM_TRACE_EVENT_PREFETCH) ? printf("P ") : (
            ((flags & QEM_TRACE_EVENT_UNLOCK) == QEM_TRACE_EVENT_UNLOCK) ? printf("U ") : (
                 ((flags & QEM_TRACE_EVENT_LOCK) == QEM_TRACE_EVENT_LOCK) ? printf("L ") : (
                         ((flags & (QEM_TRACE_EVENT_INVALIDATE | QEM_TRACE_EVENT_FLUSH)) == (QEM_TRACE_EVENT_INVALIDATE | QEM_TRACE_EVENT_FLUSH)) ? printf("FL/INV ") : (
                             ((flags & QEM_TRACE_EVENT_INVALIDATE) == QEM_TRACE_EVENT_INVALIDATE) ? printf("INV ") : (
                                 ((flags & QEM_TRACE_EVENT_FLUSH) == QEM_TRACE_EVENT_FLUSH) ? printf("FL ") : ((flags & QEM_TRACE_DATA_TYPE_INST) ? printf("I "): printf("D "))))))));


    if(flags & QEM_TRACE_ACCESS_TYPE_WRITE)
    {
        printf("ST ");
    }
    else
    {
        printf("LD ");
    }

    printf("| V 0x%08x | P 0x%08x | Core %d | Time %" PRIu64 " | RW: %c | ID: %c | PL: %c | E: %c | G: %c | S: %c | CR: %c | CL: %c | CI: %c | WT: %c | EX: %c\n",
           virt_addr, phys_addr, core, time,
           (flags & QEM_TRACE_ACCESS_TYPE_WRITE) ? 'W' : 'R',

           (flags & QEM_TRACE_DATA_TYPE_INST) ? 'I' : 'D',

           (flags & QEM_TRACE_PL_USER) ? 'U' : 'K',

           ((flags & QEM_TRACE_EVENT_MASK) == QEM_TRACE_EVENT_DCBZ) ? 'D' : (
               ((flags & QEM_TRACE_EVENT_MASK) == QEM_TRACE_EVENT_PREFETCH) ? 'P' : (
                   ((flags & QEM_TRACE_EVENT_MASK) == QEM_TRACE_EVENT_UNLOCK) ? 'U' : (
                        ((flags & QEM_TRACE_EVENT_MASK) == QEM_TRACE_EVENT_LOCK) ? 'L' : (
                                ((flags & QEM_TRACE_EVENT_MASK) == (QEM_TRACE_EVENT_INVALIDATE | QEM_TRACE_EVENT_FLUSH)) ? 'B' : (
                                    ((flags & QEM_TRACE_EVENT_MASK) == QEM_TRACE_EVENT_INVALIDATE) ? 'I' : (
                                        ((flags & QEM_TRACE_EVENT_MASK) == QEM_TRACE_EVENT_FLUSH) ? 'F' : 'A')))))),

           ((flags & QEM_TRACE_GRANULARITY_MASK) == QEM_TRACE_GRANULARITY_SET) ? 'S' :
           ((flags & QEM_TRACE_GRANULARITY_MASK) == QEM_TRACE_GRANULARITY_WAY) ? 'W' :
           ((flags & QEM_TRACE_GRANULARITY_MASK) == QEM_TRACE_GRANULARITY_GLOBAL) ? 'G' : 'L',

           ((flags & QEM_TRACE_DATA_SIZE_MASK) == QEM_TRACE_DATA_SIZE_16_BITS) ? 'H' :
           ((flags & QEM_TRACE_DATA_SIZE_MASK) == QEM_TRACE_DATA_SIZE_32_BITS) ? 'W' :
           ((flags & QEM_TRACE_DATA_SIZE_MASK) == QEM_TRACE_DATA_SIZE_64_BITS) ? 'D' : 'B',

           (flags & QEM_TRACE_COHERENCY_REQ) ? 'Y' : 'N',

           ((flags & QEM_TRACE_CACHE_LEVEL_MASK) == QEM_TRACE_CACHE_LEVEL_ALL) ? 'A' :
           ((flags & QEM_TRACE_CACHE_LEVEL_MASK) == QEM_TRACE_CACHE_LEVEL_3) ? '3' :
           ((flags & QEM_TRACE_CACHE_LEVEL_MASK) == QEM_TRACE_CACHE_LEVEL_2) ? '2' : '1',

           (flags & QEM_TRACE_CACHE_INHIBIT_ON) ? 'Y' : 'N',

           (flags & QEM_TRACE_CACHE_WT_ON) ? 'E' : 'D',

           (flags & QEM_TRACE_EVENT_EXCLUSIVE) ? 'Y' : 'N'
       );
#endif
}

#endif /* QEM_TRACE_ENABLED */
