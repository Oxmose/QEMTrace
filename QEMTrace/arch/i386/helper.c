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
#include "exec/address-spaces.h"

#include "../../qem_trace_config.h" /*QEMTrace configuration */

#if QEM_TRACE_ENABLED

#include "../../qem_trace_engine.h" /* QemTrace engine */
#include "../../qem_trace_def.h"    /* QemTrace trace definitions */
#include "qem_trace_engine.h"

/**************************** ACCESS TRACE ************************************/

static void flatview_read(FlatView *fv, hwaddr addr,
                                 MemTxAttrs attrs, uint8_t *buf, int len)
{
    hwaddr l;
    hwaddr addr1;
    MemoryRegion *mr;

    l = len;
    mr = flatview_translate(fv, addr, &addr1, &l, false, attrs);
    flatview_read_continue(fv, addr, attrs, buf, len,
                                  addr1, l, mr);
}

void helper_qem_instld_trace(CPUX86State *env, target_ulong current_eip, int size)
{
     if((qem_tracing_state & QEM_TRACE_ITRACE) != 0)
     {
        uint32_t l1, l2;
        uint32_t pgd, pde, pte;
        target_ulong phys_addr = 0;
        uint32_t cache_inhibit = 0;

        /* Check if paging is enabled */
        if((env->cr[0] & CR0_PG_MASK) == 0)
        {
            phys_addr = current_eip;
        }
        else
        {
            /* Get the level 1 entry */
            l1 = current_eip >> 22;
            /* Get the level 2 entry */
            l2 = (current_eip & 0x3FFFFF) >> 12;

            pgd = env->cr[3] & ~0xfff;

            /* Get the PDE */
            FlatView *fv;

            fv = address_space_to_flatview(&address_space_memory);

            flatview_read(fv, pgd + l1 * 4, MEMTXATTRS_UNSPECIFIED, (void*)&pde, 4);
            pde = le32_to_cpu(pde);

            /* If mapped */
            if (pde & PG_PRESENT_MASK)
            {
                /* If 4M Pages */
                if ((pde & PG_PSE_MASK) && (env->cr[4] & CR4_PSE_MASK))
                {
                    cache_inhibit = (pde & 0x00000010);
                    phys_addr = (pde & 0xFFC00000) | (current_eip & 0x003FFFFF);
                }
                /* If 4K Pages */
                else
                {
                    flatview_read(fv, (pde & ~0xfff) + l2 * 4, MEMTXATTRS_UNSPECIFIED, (void*)&pte, 4);
                    pte = le32_to_cpu(pte);

                    /* If mapped */
                    if (pte & PG_PRESENT_MASK)
                    {
                        cache_inhibit = (pte & 0x00000040) | (pde & 0x00000010);
                        phys_addr = (pte & 0xFFFFF000) | (current_eip & 0x00000FFF);
                    }
                }
            }
        }

        /* Get time, we prefer to use TSC to lowe overhead */
        unsigned a, d;
        asm volatile("rdtsc" : "=a" (a), "=d" (d) : : "%rbx", "%rcx");

        /* Get flags */
        uint32_t flags = QEM_TRACE_EVENT_ACCESS | QEM_TRACE_ACCESS_TYPE_READ |
                         QEM_TRACE_DATA_TYPE_INST | QEM_TRACE_GRANULARITY_NONE;

        /* Get inhibition state BIT 30 or cr0 */
        flags |= ((env->cr[0] & (1 << 30)) | cache_inhibit) ?
                 QEM_TRACE_CACHE_INHIBIT_ON : QEM_TRACE_CACHE_INHIBIT_OFF;

        /* To check the current ring we read CS register and check is the low 8 Bits
         * have a value of 3
         */

        flags |= ((env->segs[R_CS].selector & 0x0003) ?
                  QEM_TRACE_PL_USER : QEM_TRACE_PL_KERNEL);

        switch(size)
        {
            case 1:
                flags |= QEM_TRACE_DATA_SIZE_8_BITS;
                break;
            case 2:
                flags |= QEM_TRACE_DATA_SIZE_16_BITS;
                break;
            case 4:
                flags |= QEM_TRACE_DATA_SIZE_32_BITS;
                break;
            case 8:
                flags |= QEM_TRACE_DATA_SIZE_64_BITS;
                break;
            default:
                flags |= QEM_TRACE_DATA_SIZE_8_BITS;
                break;
        }

        qem_trace_output(current_eip, phys_addr,
                                ENV_GET_CPU(env)->cpu_index,
                                ((uint64_t) a) | (((uint64_t) d) << 32),
                                flags);
     }

}

void helper_qem_datald_trace(CPUArchState *env, target_ulong addr,
                         int32_t data_size)
{
    if((qem_tracing_state & QEM_TRACE_DTRACE) != 0)
    {
        uint32_t cache_inhibit;
        /* Get the physical address corresponding to the virtual address */
#if QEM_TRACE_PHYSICAL_ADDRESS
        target_ulong phys_addr = qem_get_phys_addr_mem_tracing(env, &cache_inhibit,
                                                           addr);
#else 
        target_ulong phys_addr = addr;
#endif
        /* Get time */
        unsigned a, d;
        asm volatile("rdtsc" : "=a" (a), "=d" (d) : : "%rbx", "%rcx");

        /* Get flags */
        uint32_t flags = qem_get_flags_mem_tracing(env,
                                               QEM_TRACE_EVENT_ACCESS,
                                               QEM_TRACE_ACCESS_TYPE_READ,
                                               QEM_TRACE_DATA_TYPE_DATA,
                                               QEM_TRACE_GRANULARITY_NONE,
                                               data_size,
                                               phys_addr);
        /* Get inhibition state BIT 30 or cr0 */
        flags |= ((env->cr[0] & (1 << 30)) | cache_inhibit) ?
                 QEM_TRACE_CACHE_INHIBIT_ON : QEM_TRACE_CACHE_INHIBIT_OFF;
                 
        /* Output trace */
        qem_trace_output(addr, phys_addr,
                               ENV_GET_CPU(env)->cpu_index,
                               ((uint64_t) a) | (((uint64_t) d) << 32),
                               flags);
    }
}

void helper_qem_datast_trace(CPUArchState *env, target_ulong addr,
                         int32_t data_size)
{
    if((qem_tracing_state & QEM_TRACE_DTRACE) != 0)
    {
        uint32_t cache_inhibit;
#if QEM_TRACE_PHYSICAL_ADDRESS
        target_ulong phys_addr = qem_get_phys_addr_mem_tracing(env, &cache_inhibit,
                                                           addr);
#else 
        target_ulong phys_addr = addr;
#endif
        /* Get time */
        unsigned a, d;
        asm volatile("rdtsc" : "=a" (a), "=d" (d) : : "%rbx", "%rcx");

        /* Get flags */
        uint32_t flags = qem_get_flags_mem_tracing(env,
                                               QEM_TRACE_EVENT_ACCESS,
                                               QEM_TRACE_ACCESS_TYPE_WRITE,
                                               QEM_TRACE_DATA_TYPE_DATA,
                                               QEM_TRACE_GRANULARITY_NONE,
                                               data_size,
                                               phys_addr);

        /* Get inhibition state BIT 30 or cr0 */
        flags |= ((env->cr[0] & (1 << 30)) | cache_inhibit) ?
                 QEM_TRACE_CACHE_INHIBIT_ON : QEM_TRACE_CACHE_INHIBIT_OFF;

        /* Output trace */
        qem_trace_output(addr, phys_addr,
                               ENV_GET_CPU(env)->cpu_index,
                               ((uint64_t) a) | (((uint64_t) d) << 32),
                               flags);
    }
}

/*********************************** MISC *************************************/

void helper_qem_start_trace(CPUX86State *env, int type)
{
    CPUState *cs = ENV_GET_CPU(env);
    qem_trace_enable(cs, (QEM_TRACE_TTYPE_E)type);
}

void helper_qem_stop_trace(int type)
{
     qem_trace_disable((QEM_TRACE_TTYPE_E)type);
}

void helper_qem_trace_start_timer(void)
{
    qem_trace_start_timer();
}

void helper_qem_trace_get_timer(void)
{
    qem_trace_get_timer();
}

#endif /* QEM_TRACE_ENABLED */