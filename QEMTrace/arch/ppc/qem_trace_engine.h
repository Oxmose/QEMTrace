/*
 * Guest memory access tracing engine for PowerPC architecture.
 *
 * Provides tools to translate virtual addresses to physical addreses.
 * Provides tools to save traces in trace files.
 *
 * Header included in
 *     qem_trace_engine.c
 *     helper.c
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
#include "mmu-hash64.h"
#include "mmu-hash32.h"
#include "mmu-book3s-v3.h"
#include "mmu-radix64.h"
#include "helper_regs.h"

#include "sysemu/kvm.h"
#include "kvm_ppc.h"

#include "qemu/error-report.h"



#include "../../qem_trace_config.h" /* QEMTrace configuration */

#if QEM_TRACE_ENABLED
#include "../../qem_trace_engine.h"

/* Context used internally during MMU translations */
typedef struct mmu_ctx_t mmu_ctx_t;
struct mmu_ctx_t {
    hwaddr raddr;      /* Real address              */
    hwaddr eaddr;      /* Effective address         */
    int prot;                      /* Protection bits           */
    hwaddr hash[2];    /* Pagetable hash values     */
    target_ulong ptem;             /* Virtual segment ID | API  */
    int key;                       /* Access key                */
    int nx;                        /* Non-execute area          */
};

void qem_ppc_get_info_addr_mem_trace(CPUPPCState *env, vaddr addr,
                                   int real_access_type,
                                   target_ulong* phys_addr,
                                   int32_t* write_through_enabled,
                                   int32_t* cache_inhibit,
                                   int32_t* coherency_enabled);



#endif /* QEM_TRACE_ENABLED */