/*
 * Guest memory access tracing engine for PowerPC architecture.
 *
 * Provides tools to translate virtual addresses to physical addreses.
 * Provides tools to save traces in trace files.
 *
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

#include "qemu/host-utils.h"
#include "cpu.h"

#include "disas/disas.h"

#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "exec/translator.h"
#include "exec/helper-proto.h"
#include "exec/helper-gen.h"
#include "exec/log.h"

#include "trace-tcg.h"
#include "tcg-op.h"

#include "sysemu/kvm.h"

#include "qemu/error-report.h"



#include "../../qem_trace_config.h" /* QEMTrace configuration */

#if QEM_TRACE_ENABLED
#include "qem_trace_engine.h"
#include "../../qem_trace_engine.h"


target_ulong qem_get_phys_addr_mem_tracing(CPUArchState* env,
                                       uint32_t* cache_inhibit,
                                       const target_ulong addr)
{
    uint32_t l1, l2;
    uint32_t pgd, pde, pte;

    /* Check if paging is enabled */
    if((env->cr[0] & CR0_PG_MASK) == 0)
    {
        *cache_inhibit = 0;
        return addr;
    }

    target_ulong haddr = 0;

    /* Get the level 1 entry */
    l1 = addr >> 22;
    /* Get the level 2 entry */
    l2 = (addr & 0x3FFFFF) >> 12;

    pgd = env->cr[3] & ~0xfff;

    /* Get the PDE */
    cpu_physical_memory_read(pgd + l1 * 4, &pde, 4);
    pde = le32_to_cpu(pde);

    /* If mapped */
    if (pde & PG_PRESENT_MASK)
    {
        /* If 4M Pages */
        if ((pde & PG_PSE_MASK) && (env->cr[4] & CR4_PSE_MASK))
        {
            *cache_inhibit = (pde & 0x00000010);
            haddr = (pde & 0xFFC00000) | (addr & 0x003FFFFF);
        }
        /* If 4K Pages */
        else
        {
            cpu_physical_memory_read((pde & ~0xfff) + l2 * 4, &pte, 4);
            pte = le32_to_cpu(pte);

            /* If mapped */
            if (pte & PG_PRESENT_MASK)
            {
                *cache_inhibit = (pte & 0x00000040) | (pde & 0x00000010);
                haddr = (pte & 0xFFFFF000) | (addr & 0x00000FFF);
            }
        }
    }
    else {
        *cache_inhibit = 0;
        haddr = addr;
    }

    return haddr;
}

uint32_t qem_get_flags_mem_tracing(const CPUArchState* env,
                               const QEM_TRACE_EVENT_E trace_type,
                               const QEM_TRACE_ACCESS_TYPE_E access_type,
                               const QEM_TRACE_DATA_SIZE_E data_type,
                               const QEM_TRACE_GRANULARITY_E granularity,
                               const uint32_t data_size,
                               const target_ulong phys_addr)
{
    uint32_t flags = access_type | data_type | trace_type | granularity;

    /* To check the current ring we read CS register and check is the low 8 Bits
     * have a value of 3
     */
    flags |= ((env->segs[R_CS].selector & 0x0003) ?
              QEM_TRACE_PL_USER : QEM_TRACE_PL_KERNEL);

    switch(data_size)
    {
        case 0:
            flags |= QEM_TRACE_DATA_SIZE_8_BITS;
            break;
        case 1:
            flags |= QEM_TRACE_DATA_SIZE_16_BITS;
            break;
        case 2:
            flags |= QEM_TRACE_DATA_SIZE_32_BITS;
            break;
        case 3:
            flags |= QEM_TRACE_DATA_SIZE_64_BITS;
            break;
        default:
            flags |= QEM_TRACE_DATA_SIZE_8_BITS;
            break;
    }

    /* Protection level */
    return flags;
}


#endif /* QEM_TRACE_ENABLED */