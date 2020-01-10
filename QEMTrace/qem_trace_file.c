/*
 * Guest memory access tracing engine.
 * 
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

#if QEM_TRACE_TYPE == QEM_TRACE_FILE

#if QEM_TRACE_MT_BUFFER_EN == 0

#include "qem_trace_engine.h" /* Engine header */
#include "qem_trace_logger.h" /* QEM logger */

/*******************************************************************************
 * GLOBAL VARS
 ******************************************************************************/
/* Keeps track on the tracing state */
volatile uint8_t qem_tracing_state = 0;

/* Keeps track execution time */
volatile uint64_t qem_time_start = 0;

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

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
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
    if(error != 1)
    {
        QEM_TRACE_ERROR("Could not write the header", error, 1);
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
    sprintf(filename, "qem_trace_%08d.out", qem_file_index++);
    filename[22] = 0;
    remove(filename);

    qem_trace_file_fd = fopen(filename, "wb+");

    /* In case of error */
    if(qem_trace_file_fd == NULL)
    {
        QEM_TRACE_ERROR("Could not create or open the trace file", errno, 1);
    }

    /* Init the header space */
    qem_write_header(0, sizeof(qem_trace_t), 1);
    qem_file_size = sizeof(qem_trace_t);

    QEM_TRACE_INFO("==== Trace file name", 0);
    QEM_TRACE_INFO(filename, 0);
}

static void qem_close_tracing(void)
{
    int error;

    /* Check buffer state and flush */
    if(qem_trace_buffer_size != 0)
    {
        error = fwrite(qem_trace_buffer, sizeof(qem_trace_t),
                       qem_trace_buffer_size, qem_trace_file_fd);

        if(error != qem_trace_buffer_size)
        {
            QEM_TRACE_ERROR("Could not flush trace buffer", error, 1);
        }

        qem_file_size += qem_trace_buffer_size * sizeof(qem_trace_t);

        qem_trace_buffer_size = 0;
    }

    qem_write_header(qem_file_size, sizeof(qem_trace_t), 1);

    /* Close file */
    if(fclose(qem_trace_file_fd) < 0)
    {
        QEM_TRACE_ERROR("Error while closing the trace file", errno, 0);
    }
    else
    {
        QEM_TRACE_INFO("Trace file saved", 0);
    }
}

void qem_trace_enable(CPUState* cs, const QEM_TRACE_TTYPE_E type)
{
    if(qem_tracing_state == type)
    {
        return;
    }

    if(qem_tracing_state == 0)
    {
        /* Open output file */
        qem_init_tracing();
    }

    /* Enable tracing state */
    qem_tracing_state = type;
    
}

void qem_trace_disable(const QEM_TRACE_TTYPE_E type)
{
    if(qem_tracing_state == 0)
    {
        return;
    }

    /* Enable tracing state */
    qem_tracing_state &= ~type;

    if(qem_tracing_state == 0)
    {
        /* Close output file or stream */
        qem_close_tracing();
    }
}

void qem_trace_start_timer(void)
{
    /* Save the current time */
    qem_time_start = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);
}

void qem_trace_get_timer(void)
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

        if(error != QEM_TRACE_BUFFER_SIZE)
        {
            QEM_TRACE_ERROR("Could not flush trace buffer", error, 1);
        }

        qem_file_size += QEM_TRACE_BUFFER_SIZE * sizeof(qem_trace_t);

        qem_trace_buffer_size = 0;
     }
     
#if QEM_TRACE_GATHER_META
     /* Create the structure */
     qem_trace_t new_trace = {
         .address = phys_addr,
         .timestamp = time,
         .flags = (core & 0xFF) | (flags & 0xFFFFFF00)
     };
#else 
    qem_trace_t new_trace = {
         .address = phys_addr,
         .flags = (uint8_t)flags
     };
#endif

     /* Copy the structure to the buffer */
     uint64_t offset = qem_trace_buffer_size * sizeof(qem_trace_t);
     memcpy(qem_trace_buffer + offset, &new_trace, sizeof(qem_trace_t));
     ++qem_trace_buffer_size;
}

#endif /* QEM_TRACE_MT_BUFFER_EN == 0 */

#endif /* QEM_TRACE_TYPE == QEM_TRACE_FILE */

#endif /* QEM_TRACE_ENABLED */
