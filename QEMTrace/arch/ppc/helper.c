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
coherency, size)                                                               \
{                                                                              \
    /* Get time, we prefer to use TSC to lower overhead */                     \
    asm volatile("rdtsc" : "=a" (t_hi), "=d" (t_lo) : : "%rbx", "%rcx");       \
                                                                               \
    /* To check the current ring we read MSR register */                       \
    flags |= ((env->msr & (1 << 17)) ?                                         \
              QEM_TRACE_PL_USER : QEM_TRACE_PL_KERNEL);                        \
                                                                               \
    switch(size)                                                               \
    {                                                                          \
         case 1:                                                               \
             flags |= QEM_TRACE_DATA_SIZE_8_BITS;                              \
             break;                                                            \
         case 2:                                                               \
             flags |= QEM_TRACE_DATA_SIZE_16_BITS;                             \
             break;                                                            \
         case 4:                                                               \
             flags |= QEM_TRACE_DATA_SIZE_32_BITS;                             \
             break;                                                            \
         case 8:                                                               \
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

void helper_qem_instld_trace(CPUPPCState *env, target_ulong current_eip,
                         int size)
{
    if(qem_tracing_enabled == 1)
    {
        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, current_eip, ACCESS_CODE, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_ACCESS | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_INST;

        unsigned a, d;

        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(current_eip,
                              haddr,
                              ENV_GET_CPU(env)->cpu_index,
                              ((uint64_t) a) | (((uint64_t) d) << 32),
                              flags);
    }
}

void helper_qem_datast_trace_direct(CPUPPCState *env, target_ulong virt_addr,
                                int size)
{
    if(qem_tracing_enabled == 1)
    {
        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_ACCESS | QEM_TRACE_ACCESS_TYPE_WRITE |
                         QEM_TRACE_DATA_TYPE_DATA;

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_datald_trace_direct(CPUPPCState *env, target_ulong virt_addr,
                                int size)
{
    if(qem_tracing_enabled == 1)
    {
        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_ACCESS | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_DATA;

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_datast_trace(CPUPPCState *env, target_ulong simm,
                         int reg, int size)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg] + simm;
        helper_qem_datast_trace_direct(env, virt_addr, size);
    }
}

void helper_qem_datast_trace_trad(CPUPPCState *env, int reg0,
                              int reg1, int size)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];
        helper_qem_datast_trace_direct(env, virt_addr, size);
    }
}

void helper_qem_datald_trace(CPUPPCState *env, target_ulong simm,
                         int reg, int size)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg] + simm;
        helper_qem_datald_trace_direct(env, virt_addr, size);
    }
}

void helper_qem_datald_trace_trad(CPUPPCState *env, int reg0,
                              int reg1, int size)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];
        helper_qem_datald_trace_direct(env, virt_addr, size);
    }
}

/*********************************** MISC *************************************/

void helper_qem_start_trace(CPUPPCState *env)
{
    CPUState *cs = ENV_GET_CPU(env);
    qem_trace_enable(cs);
}

void helper_qem_stop_trace(void)
{
     qem_trace_disable();
}

void helper_qem_trace_start_timer(void)
{
    qem_trace_start_timer();
}

void helper_qem_trace_get_timer(void)
{
    qem_trace_get_timer();
}

/******************************* CACHES TRACE *********************************/

void helper_qem_dcache_flush_inval(CPUPPCState *env, int reg0, int reg1, int level,
                               int granularity)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_FLUSH | QEM_TRACE_EVENT_INVALIDATE |
                         QEM_TRACE_DATA_TYPE_DATA |
                         (QEM_TRACE_GRANULARITY_E)granularity;

        switch(level)
        {
            case 0:
                flags |= QEM_TRACE_CACHE_LEVEL_1;
                break;
            case 1:
                flags |= QEM_TRACE_CACHE_LEVEL_2;
                break;
            case 2:
                flags |= QEM_TRACE_CACHE_LEVEL_3;
                break;
            default:
                flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                break;
        }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_dcache_inval(CPUPPCState *env, int reg0, int reg1, int level,
                         int granularity)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_INVALIDATE |
                         QEM_TRACE_DATA_TYPE_DATA |
                         (QEM_TRACE_GRANULARITY_E)granularity;

         switch(level)
         {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
         }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_dcache_flush(CPUPPCState *env, int reg0, int reg1, int level,
                         int granularity, int exclusive)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_FLUSH |
                         QEM_TRACE_DATA_TYPE_DATA |
                         (QEM_TRACE_GRANULARITY_E)granularity;

        if(exclusive == 1)
        {
            flags |= QEM_TRACE_EVENT_EXCLUSIVE;
        }

         switch(level)
         {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
         }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_dcache_prefetch_non_inibited(CPUPPCState *env, int reg0, int reg1,
                                         int level, int granularity,
                                         int exclusive)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        uint32_t size = 1;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_PREFETCH | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_DATA;

        if(exclusive == 1)
        {
            flags |= QEM_TRACE_EVENT_EXCLUSIVE;
        }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_dcache_lock(CPUPPCState *env, int reg0, int reg1, int level,
                        int granularity, int exclusive)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_LOCK |
                         QEM_TRACE_DATA_TYPE_DATA |
                         (QEM_TRACE_GRANULARITY_E)granularity;
        if(exclusive == 1)
        {
            flags |= QEM_TRACE_EVENT_EXCLUSIVE;
        }
        switch(level)
        {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
        }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_dcache_unlock(CPUPPCState *env, int reg0, int reg1, int level,
                          int granularity)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_UNLOCK |
                         QEM_TRACE_DATA_TYPE_DATA |
                         (QEM_TRACE_GRANULARITY_E)granularity;

        switch(level)
        {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
        }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_icache_inval(CPUPPCState *env, int reg0, int reg1, int level,
                         int granularity)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_CODE, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_INVALIDATE |
                         QEM_TRACE_DATA_TYPE_INST |
                         (QEM_TRACE_GRANULARITY_E)granularity;

         switch(level)
         {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
         }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_icache_flush(CPUPPCState *env, int reg0, int reg1, int level,
                         int granularity)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_CODE, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_FLUSH |
                         QEM_TRACE_DATA_TYPE_INST |
                         (QEM_TRACE_GRANULARITY_E)granularity;

         switch(level)
         {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
         }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}
void helper_qem_icache_lock(CPUPPCState *env, int reg0, int reg1, int level,
                        int granularity, int exclusive)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_CODE, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_LOCK |
                         QEM_TRACE_DATA_TYPE_INST |
                         (QEM_TRACE_GRANULARITY_E)granularity;

        if(exclusive == 1)
        {
            flags |= QEM_TRACE_EVENT_EXCLUSIVE;
        }

        switch(level)
        {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
        }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_icache_unlock(CPUPPCState *env, int reg0, int reg1, int level,
                          int granularity)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        uint32_t size = 1;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_CODE, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_UNLOCK |
                         QEM_TRACE_DATA_TYPE_INST |
                         (QEM_TRACE_GRANULARITY_E)granularity;
        switch(level)
        {
             case 0:
                 flags |= QEM_TRACE_CACHE_LEVEL_1;
                 break;
             case 1:
                 flags |= QEM_TRACE_CACHE_LEVEL_2;
                 break;
             case 2:
                 flags |= QEM_TRACE_CACHE_LEVEL_3;
                 break;
             default:
                 flags |= QEM_TRACE_CACHE_LEVEL_ALL;
                 break;
        }

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_icbt(CPUPPCState *env, int reg0, int reg1, int level)
{
    if(qem_tracing_enabled == 1)
    {
        
        /* ICBT acts like a load only on CT = 1 */
        if(level != 1)
        {
            return;
        }

        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        uint32_t size = 1;

        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_CODE, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_PREFETCH | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_INST | QEM_TRACE_CACHE_LEVEL_2;

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_dcbz_trace_trad(CPUPPCState *env, int reg0, int reg1, int size)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = env->gpr[reg0] + env->gpr[reg1];

        /* Get information about the adreess */
        target_ulong haddr;
        int32_t coherency_enabled;
        int32_t cache_inhibit;
        int32_t wt_enable;
        ppc_get_info_addr_mem_trace(env, virt_addr, ACCESS_INT, &haddr,
                                    &wt_enable,
                                    &cache_inhibit, &coherency_enabled);

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_DCBZ | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_DATA;

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_flash_inval_icache(CPUPPCState *env)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = 0;

        /* Get information about the adreess */
        target_ulong haddr = 0;
        int32_t coherency_enabled = 0;
        int32_t cache_inhibit = 0;
        uint32_t size = 1;
        int32_t wt_enable = 0;

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_INVALIDATE |
                         QEM_TRACE_DATA_TYPE_INST |
                         QEM_TRACE_GRANULARITY_GLOBAL | QEM_TRACE_CACHE_LEVEL_1;

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_flash_inval_dcache(CPUPPCState *env)
{
    if(qem_tracing_enabled == 1)
    {
        target_ulong virt_addr = 0;

        /* Get information about the adreess */
        target_ulong haddr = 0;
        int32_t coherency_enabled = 0;
        int32_t cache_inhibit = 0;
        uint32_t size = 1;
        int32_t wt_enable = 0;

        /* Set flags */
        uint32_t flags = QEM_TRACE_EVENT_INVALIDATE |
                         QEM_TRACE_DATA_TYPE_DATA |
                         QEM_TRACE_GRANULARITY_GLOBAL | QEM_TRACE_CACHE_LEVEL_1;

        unsigned a, d;
        GET_ENV_FLAGS(flags, a, d, wt_enable, cache_inhibit,
                      coherency_enabled, size)

        qem_trace_output(virt_addr,
                                haddr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
    }
}

void helper_qem_printf_env_spr(CPUPPCState *env, int sprn)
{
    #if defined TARGET_PPC64
    printf("SPRN %d\n", sprn);
    printf("\t0x%016lx\n", env->spr[sprn]);
    #else
    printf("SPRN %d\n", sprn);
    printf("\t0x%016ux\n", env->spr[sprn]);
    #endif
}
void helper_qem_printf_env_gpr(CPUPPCState *env, int gprn)
{
    #if defined TARGET_PPC64
    printf("GPRN %d\n", gprn);
    printf("\t0x%016lx\n", env->gpr[gprn]);
    #else
    printf("GPRN %d\n", gprn);
    printf("\t0x%016ux\n", env->gpr[gprn]);
    #endif
}

#endif /* QEM_TRACE_ENABLED */