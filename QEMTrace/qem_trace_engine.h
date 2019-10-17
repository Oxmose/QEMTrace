/*
 * Guest memory access tracing engine.
 *
 * Provides tools to translate virtual addresses to physical addreses.
 * Provides tools to save traces in trace files.
 *
 * Header included in
 *     accel/tcg/softmmu_template.h
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

#ifndef __QEM_TRACE_ENGINE_H_
#define __QEM_TRACE_ENGINE_H_

#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

#include <stdint.h>        /* Generic types */   
#include "cpu.h"           /* Qemu CPU types */
#include "qem_trace_def.h" /* QEM Trace definition */


/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

#if QEM_TRACE_TARGET_64
typedef struct qem_trace
{
    uint64_t address;
    uint64_t timestamp;
    uint32_t  flags;
}__attribute__((packed)) qem_trace_t;
#else
typedef struct qem_trace
{
    uint32_t address;
    uint64_t timestamp;
    uint32_t  flags;
}__attribute__((packed)) qem_trace_t;
#endif

typedef struct qem_trace_header
{
    uint64_t size;
    uint16_t struct_size;
    uint8_t  version;
    uint8_t  reserved[5];
}__attribute__((packed)) qem_trace_header_t;

/* Keeps track on the tracing state */
extern volatile uint32_t qem_tracing_enabled;

/* Keeps track execution time */
extern volatile uint64_t qem_time_start;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Enables Qemu guest memory accesses tracing. The function set the tracing
 * state to 1 and flush all the software TLB to avoid Qemu fast path from
 * hidding memory access.
 *
 * @param cs The current CPU state, used to flush the software TLB.
 */
void qem_trace_enable(CPUState* cs);

/* Disables Qemu guest memory accesses tracing. */
void qem_trace_disable(void);

/* Starts emulation time counter */
void qem_trace_start_timer(void);

/* Returns the emulation time elapsed since the last start point */
void qem_trace_get_timer(void);


/* Output a trace for a memory event. Depending on the options the
 * output will be done on the standard output or a speific binary file.
 *
 * @param virt_addr The virtual address concerned by the memory access.
 * @param phys_addr The physical address concerned by the memory access.
 * @param core The core on which the memory event occured.
 * @param time The time unit aat which the memory event occured.
 * @param flags The flags describing the memory access.
 */
#if QEM_TRACE_TARGET_64
void qem_trace_output(uint64_t virt_addr, uint64_t phys_addr,
                      uint32_t core, uint64_t time, uint32_t flags);
#else
void qem_trace_output(uint32_t virt_addr, uint32_t phys_addr,
                      uint32_t core, uint64_t time, uint32_t flags);
#endif

/*******************************************************************************
 * THE FOLLOWING FUNCTIONS HAVE TO BE IMPLEMENTED FOR EACH ARCHITECTURE
 ******************************************************************************/

/* Returns the physical address conresponding to the virtual address given as
 * parameter.
 *
 * @param env  The current CPU environment.
 * @param cache_inhibit Out parameter, telling the cache inhibition state of
 * the page
 * @param addr The virtual address to be translated.
 * @returns The function return the translated virtual address to physiscal
 * address.
 */
target_ulong qem_trace_get_phys_addr_(CPUArchState* env,
                                      uint32_t* cache_inhibit,
                                      const target_ulong addr);

/* Returns the tracing flags as defined by the trace file description document.
 *
 * @param env The current CPU environment.
 * @param trace_type The trace type, see CONSTANTS section for more information.
 * @param access_type The access type, see CONSTANTS section for more
 * information.
 * @param data_type The data type, see CONSTANTS section for more information.
 * @param phys_addr The physical address of the acces, may be 0, in this case
 * the access is considered non mapped.
 * @param granularity The granularity of the request trace, 0 if the trace is an
 * not a request, like an access.
 * @returns The function returns the formated flag field of the trace.
 */
uint32_t qem_trace_get_flags(const CPUArchState* env,
                             const QEM_TRACE_EVENT_E trace_type,
                             const QEM_TRACE_ACCESS_TYPE_E access_type,
                             const QEM_TRACE_DATA_SIZE_E data_type,
                             const QEM_TRACE_GRANULARITY_E granularity,
                             const uint32_t data_size,
                             const target_ulong phys_addr);

#endif /* QEM_TRACE_ENABLED */

#endif /* __QEM_TRACE_ENGINE_H_ */
