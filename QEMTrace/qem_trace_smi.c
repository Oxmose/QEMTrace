/*
 * Guest memory access tracing engine.
 *
 * Provides tools to export traces by via the shared memory interface.
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

#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

#if QEM_TRACE_TYPE == QEM_TRACE_SMI

#include <stdlib.h>

#include "qemu/osdep.h"
#include "qemu/host-utils.h"
#include "cpu.h"
#include "qemu/timer.h"

#include "qem_trace_smi_engine.h" /* SMI engine */
#include "qem_trace_engine.h"     /* Engine header */
#include "qem_trace_logger.h"     /* QEM logger */
#include "qem_trace_def.h"        /* Trace format */

/*******************************************************************************
 * GLOBAL VARS
 ******************************************************************************/
/* Keeps track on the tracing state */
volatile uint8_t qem_tracing_state = 0;

/* Keeps track execution time */
volatile uint64_t qem_time_start = 0;

/* SMI connection state */
static uint8_t connected = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void qem_init_tracing(void)
{
    int32_t error = 0;
    if(connected == 0)
    {
        qem_smi_client_init();
        error = qem_smi_client_connect(1, 1000);
        /* Connect the SMI */
        if(error != 0)
        {
            QEM_TRACE_ERROR("[QEMU] Error while connecting SMI (step 1)", 
                            error, 1);
        }

        connected = 1;
    }
    
    /* Sending start tracing stream data sequence */
    uint32_t buffer;

    buffer = QEM_SMI_STREAM_VERSION;
    error = qem_smi_client_send(&buffer, sizeof(uint32_t));
    if(error != 0)
    {
        QEM_TRACE_ERROR("Error while connecting SMI (step 2)", 
                        error, 1);
    }

    buffer = sizeof(qem_trace_t);
    error = qem_smi_client_send(&buffer, sizeof(uint32_t));
    if(error != 0)
    {
        QEM_TRACE_ERROR("Error while connecting SMI (step 3)", 
                        error, 1);
    }
    
    QEM_TRACE_INFO("Init SMI sequence sent", 0);
}

static void qem_close_tracing(void)
{
    /* Send end stream sequence */
    int32_t error;
    uint8_t buffer[sizeof(qem_trace_t)] = {0};
    error = qem_smi_client_send(buffer, sizeof(qem_trace_t));
    
    if(error != 0)
    {
        QEM_TRACE_ERROR("Error while closing SMI (step 3)", 
                        error, 1);
    }

    qem_smi_client_flush();
    
    QEM_TRACE_INFO("END SMI sequence sent", 0);
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
    int32_t error;

    /* Create the structure */
    qem_trace_t new_trace = {
        .address = phys_addr,
        .timestamp = time,
        .flags = (core & 0xFF) | (flags & 0xFFFFFF00)
    };

    /* Send the structure */
    error = qem_smi_client_send(&new_trace, sizeof(qem_trace_t));

    if(error != 0)
    {
        QEM_TRACE_ERROR("Could not send trace with SMI", error, 1);
    }
}

#endif /* QEM_TRACE_TYPE == QEM_TRACE_SMI */

#endif /* QEM_TRACE_ENABLED */
