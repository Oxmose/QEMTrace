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
#include <pthread.h>

#include "qemu/osdep.h"
#include "qemu/host-utils.h"
#include "cpu.h"
#include "qemu/timer.h"

#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

#if QEM_TRACE_TYPE == QEM_TRACE_FILE

#if QEM_TRACE_MT_BUFFER_EN != 0

#include "qem_trace_engine.h" /* Engine header */
#include "qem_trace_logger.h" /* QEM logger */

/*******************************************************************************
 * GLOBAL VARS
 ******************************************************************************/
/* Keeps track on the tracing state */
volatile uint32_t qem_tracing_enabled = 0;

/* Keeps track execution time */
volatile uint64_t qem_time_start = 0;

/* This is used to implement the write buffer. This buffer contains the last
 * memory traces to be written back to the disk. Using the buffer allows to
 * avoid having to call "write" at avery tracing point and allow a nearly-null
 * overhead when writing to the file.
 */
static uint8_t  qem_trace_current_buffer = 0;
static uint32_t qem_trace_buffer_sizes[2] = {0};
static uint8_t  qem_trace_buffer[2][QEM_TRACE_BUFFER_SIZE * sizeof(qem_trace_t)];
static uint64_t qem_file_size = 0;
static FILE*    qem_trace_file_fd;

/* Synchronization between the two trace buffers */
static sem_t   qem_trace_buffer_sync;
static uint8_t qem_trace_buffer_sync_init = 0; 

/* Flushing thread */
static pthread_t qem_trace_flush_thread;

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

    /* Wait for the last buffer to be flushed */
    error = sem_wait(&qem_trace_buffer_sync);
    if(error != 0)
    {
        QEM_TRACE_ERROR("Could not get synchronization semaphore", error, 1);
    }

    /* Write the header */
    error = fwrite(&header, sizeof(qem_trace_header_t),
                   1, qem_trace_file_fd);
    if(error != 1)
    {
        QEM_TRACE_ERROR("Could not write the header", error, 1);
    }

    /* Reset the buffer size */
    qem_trace_buffer_sizes[qem_trace_current_buffer] = 0;

    error = sem_post(&qem_trace_buffer_sync);
    if(error != 0)
    {
        QEM_TRACE_ERROR("Could not post synchronization semaphore", error, 1);
    }
}

static void qem_init_tracing(void)
{
    int error;
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

    /* Init the synchronization semaphore */
    if(qem_trace_buffer_sync_init == 0)
    {
        error = sem_init(&qem_trace_buffer_sync, 0, 1);
        if(error != 0)
        {
            QEM_TRACE_ERROR("Could not initialize synchronization semaphore", 
                             error, 1);
        }
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

    /* Wait for the last buffer to be flushed */
    error = sem_wait(&qem_trace_buffer_sync);
    if(error != 0)
    {
        QEM_TRACE_ERROR("Could not get synchronization semaphore", error, 1);
    }

    /* Check buffer state and flush */
    if(qem_trace_buffer_sizes[qem_trace_current_buffer] != 0)
    {
        error = fwrite(qem_trace_buffer[qem_trace_current_buffer], 
                       sizeof(qem_trace_t),
                       qem_trace_buffer_sizes[qem_trace_current_buffer], 
                       qem_trace_file_fd);

        if(error != qem_trace_buffer_sizes[qem_trace_current_buffer])
        {
            QEM_TRACE_ERROR("Could not flush trace buffer", error, 1);
        }

        qem_file_size += qem_trace_buffer_sizes[qem_trace_current_buffer] * 
                         sizeof(qem_trace_t);

        qem_trace_buffer_sizes[qem_trace_current_buffer] = 0;
    }

    /* Release the semaphore */
    error = sem_post(&qem_trace_buffer_sync);
    if(error != 0)
    {
        QEM_TRACE_ERROR("Could not post synchronization semaphore", error, 1);
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

static void* qem_trace_flush_routine(void* arg)
{
    int error;
    uint8_t last_buffer = (uint8_t)arg;

    error = fwrite(qem_trace_buffer[last_buffer], sizeof(qem_trace_t),
                   qem_trace_buffer_sizes[last_buffer], qem_trace_file_fd);

    if(error != qem_trace_buffer_sizes[last_buffer])
    {
        QEM_TRACE_ERROR("Could not flush trace buffer", error, 1);
    }

    qem_file_size += qem_trace_buffer_sizes[last_buffer] * 
                        sizeof(qem_trace_t);

    qem_trace_buffer_sizes[last_buffer] = 0;

    /* Release the semaphore */
    error = sem_post(&qem_trace_buffer_sync);
    if(error != 0)
    {
        QEM_TRACE_ERROR("Could not post synchronization semaphore", error, 1);
    }

    return NULL;
}

void qem_trace_enable(CPUState* cs)
{
    if(qem_tracing_enabled == 1)
    {
        return;
    }

    /* Enable tracing state */
    qem_tracing_enabled = 1;

    /* Open output file */
    qem_init_tracing();
}

void qem_trace_disable(void)
{
    if(qem_tracing_enabled == 0)
    {
        return;
    }

    /* Enable tracing state */
    qem_tracing_enabled = 0;

    /* Close output file or stream */
    qem_close_tracing();
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
    uint8_t last_buffer;

    (void)virt_addr;
    /* Note that these functions are called at each instruction, this is why we
     * do not perform any test on the file descriptor nor the write execution
     * state.
     */

    /* The buffer size is a multiple of the trace size so we just check if we
     * can add a new entry */
     if(qem_trace_buffer_sizes[qem_trace_current_buffer] == 
        QEM_TRACE_BUFFER_SIZE)
     {
        last_buffer = qem_trace_current_buffer;

        /* Wait for next buffer to be ready */
        error = sem_wait(&qem_trace_buffer_sync);
        if(error != 0)
        {
            QEM_TRACE_ERROR("Could not get synchronization semaphore", error, 1);
        }

        /* Create the writing thread */
        error = pthread_create(&qem_trace_flush_thread, NULL, 
                               qem_trace_flush_routine, (void *) last_buffer);
        if(error != 0)
        {
            QEM_TRACE_ERROR("Could not create flush trace thread", error, 1);
        }
        error = pthread_detach(qem_trace_flush_thread);
        if(error != 0)
        {
            QEM_TRACE_ERROR("Could not detach flush trace thread", error, 1);
        }

        /* Switch current buffer */
        qem_trace_current_buffer = (qem_trace_current_buffer + 1) % 2;
     }

     /* Create the structure */
     qem_trace_t new_trace = {
         .address = phys_addr,
         .timestamp = time,
         .flags = (core & 0xFF) | (flags & 0xFFFFFF00)
     };

     /* Copy the structure to the buffer */
     uint64_t offset = qem_trace_buffer_sizes[qem_trace_current_buffer] * 
                       sizeof(qem_trace_t);
     memcpy(qem_trace_buffer[qem_trace_current_buffer] + offset, &new_trace, 
            sizeof(qem_trace_t));
     ++qem_trace_buffer_sizes[qem_trace_current_buffer];
}

#endif /* QEM_TRACE_MT_BUFFER_EN != 0 */

#endif /* QEM_TRACE_TYPE == QEM_TRACE_FILE */

#endif /* QEM_TRACE_ENABLED */
