/*
 * Guest memory access tracing for output printing.
 *
 * Provides tools to output tracing to the standard output stream.
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

#if QEM_TRACE_TYPE == QEM_TRACE_PRINT

#include "qem_trace_engine.h" /* Engine header */
#include "qem_trace_logger.h" /* QEM logger */

/*******************************************************************************
 * GLOBAL VARS
 ******************************************************************************/
/* Keeps track on the tracing state */
volatile uint8_t qem_tracing_state = 0;

/* Keeps track execution time */
volatile uint64_t qem_time_start = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void qem_trace_enable(CPUState* cs, const QEM_TRACE_TTYPE_E type)
{
    /* Enable tracing state */
    qem_tracing_state = type;
}

void qem_trace_disable(const QEM_TRACE_TTYPE_E type)
{
    /* Enable tracing state */
    qem_tracing_state &= ~type;
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
}

#endif /* QEM_TRACE_TYPE == QEM_TRACE_PRINT */

#endif /* QEM_TRACE_ENABLED */
