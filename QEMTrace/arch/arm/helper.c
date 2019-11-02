/*
 * QEM Trace additionnal QEMU helpers.
 * This is used during code generation.
 *
 * Provides QEMTrace the configuration needed to compile Qemu with the 
 * proper set of tools.
 * 
 * Header included in
 *     accel/tcg/qem_trace_engine.h
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

#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/exec-all.h"
#include "exec/helper-proto.h"

#include "../../qem_trace_config.h" /*QEMTrace configuration */

#if QEM_TRACE_ENABLED

#include "../../qem_trace_engine.h" /* QemTrace engine */
#include "../../qem_trace_def.h"    /* QemTrace trace definitions */
#include "qem_trace_engine.h"

#define GET_ENV_FLAGS(flags, t_hi, t_lo, wt_enable, cache_inhibit,             \
coherency, size, mode)                                                         \
{                                                                              \
    /* Get time, we prefer to use TSC to lower overhead */                     \
    asm volatile("rdtsc" : "=a" (t_hi), "=d" (t_lo) : : "%rbx", "%rcx");       \
                                                                               \
    /* To check the current ring we read MSR register */                       \
    flags |= (mode ? QEM_TRACE_PL_KERNEL : QEM_TRACE_PL_USER);                 \
                                                                               \
    switch(size)                                                               \
    {                                                                          \
         case 0:                                                               \
             flags |= QEM_TRACE_DATA_SIZE_8_BITS;                              \
             break;                                                            \
         case 1:                                                               \
             flags |= QEM_TRACE_DATA_SIZE_16_BITS;                             \
             break;                                                            \
         case 2:                                                               \
             flags |= QEM_TRACE_DATA_SIZE_32_BITS;                             \
             break;                                                            \
         case 3:                                                               \
             flags |= QEM_TRACE_DATA_SIZE_64_BITS;                             \
             break;                                                            \
         default:                                                              \
             flags |= QEM_TRACE_DATA_SIZE_8_BITS;                              \
             break;                                                            \
    }                                                                          \
                                                                               \
    flags |= ((coherency == 1) ?                                               \
             QEM_TRACE_COHERENCY_REQ : QEM_TRACE_COHERENCY_NON_REQ);           \
    flags |= ((cache_inhibit == 1) ?                                           \
             QEM_TRACE_CACHE_INHIBIT_ON : QEM_TRACE_CACHE_INHIBIT_OFF);        \
    flags |= ((wt_enable == 1) ?                                               \
             QEM_TRACE_CACHE_WT_ON : QEM_TRACE_CACHE_WT_OFF);                  \
}

/**************************** ACCESS TRACE ************************************/

void helper_qem_instld_trace(CPUARMState *env, target_ulong current_eip,
                             int size, int mmu_idx)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong hwaddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        int32_t access_mode;
        qem_arm_get_info_addr_mem_trace(env, current_eip, MMU_INST_FETCH, 
                                    (ARMMMUIdx)mmu_idx,
                                    &hwaddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled, &access_mode);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_ACCESS | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_INST;

        unsigned a, d;

        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size, access_mode);
     
        qem_trace_output(current_eip,
                         hwaddr,
                         ENV_GET_CPU(env)->cpu_index,
                         ((uint64_t) a) | ((uint64_t) d) << 32,
                         flags);
    }
}

void helper_qem_datald_trace(CPUARMState *env, target_ulong addr,
                             int size, int mmu_idx)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong hwaddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        int32_t access_mode;
        qem_arm_get_info_addr_mem_trace(env, addr, MMU_DATA_LOAD, 
                                    (ARMMMUIdx)mmu_idx,
                                    &hwaddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled, &access_mode);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_ACCESS | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_DATA;

        unsigned a, d;

        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size, access_mode);
                      
        qem_trace_output(addr,
                         hwaddr,
                         ENV_GET_CPU(env)->cpu_index,
                         ((uint64_t) a) | ((uint64_t) d) << 32,
                         flags);
    }
}

void helper_qem_datast_trace(CPUARMState *env, target_ulong addr,
                             int size, int mmu_idx)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong hwaddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        int32_t access_mode;
        qem_arm_get_info_addr_mem_trace(env, addr, MMU_DATA_STORE, 
                                    (ARMMMUIdx)mmu_idx,
                                    &hwaddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled, &access_mode);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_ACCESS | QEM_TRACE_ACCESS_TYPE_WRITE |
                         QEM_TRACE_DATA_TYPE_DATA;

        unsigned a, d;

        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size, access_mode);
                      
        qem_trace_output(addr,
                         hwaddr,
                         ENV_GET_CPU(env)->cpu_index,
                         ((uint64_t) a) | ((uint64_t) d) << 32,
                         flags);
    }
}

/*********************************** MISC *************************************/

void helper_qem_start_trace(CPUARMState *env)
{
    CPUState *cs = ENV_GET_CPU(env);
    qem_trace_enable(cs);
}

void helper_qem_stop_trace(void)
{
     qem_trace_disable();
}
#endif /* QEM_TRACE_ENABLED */