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
#include "qem_trace_engine.h"
#include "../../qem_trace_engine.h"

/* Internal QEMU functions */
int mmubooke_get_physical_address(CPUPPCState *env, mmu_ctx_t *ctx,
                                         target_ulong address, int rw,
                                         int access_type);
int check_physical(CPUPPCState *env, mmu_ctx_t *ctx,
                                 target_ulong eaddr, int rw);
int get_bat_6xx_tlb(CPUPPCState *env, mmu_ctx_t *ctx,
                           target_ulong virtual, int rw, int type);
int get_segment_6xx_tlb(CPUPPCState *env, mmu_ctx_t *ctx,
                                      target_ulong eaddr, int rw, int type); 
int mmubooke206_check_tlb(CPUPPCState *env, ppcmas_tlb_t *tlb,
                                 hwaddr *raddr, int *prot,
                                 target_ulong address, int rw,
                                 int access_type, int mmu_idx);
int mmu40x_get_physical_address(CPUPPCState *env, mmu_ctx_t *ctx,
                                       target_ulong address, int rw,
                                       int access_type);
                                       
static int mmubooke206_get_info(CPUPPCState *env, mmu_ctx_t *ctx,
                                   target_ulong address, int rw,
                                   int access_type,
                                   int real_access_type,
                                   int32_t* write_through_enabled,
                                   int32_t* cache_inhibit,
                                   int32_t* coherency_enabled)
{
    ppcmas_tlb_t *tlb;
    hwaddr raddr;
    int i, j, ret;

    ret = -1;
    raddr = (hwaddr)-1ULL;

    for (i = 0; i < BOOKE206_MAX_TLBN; i++) {
        int ways = booke206_tlb_ways(env, i);

        for (j = 0; j < ways; j++) {
            tlb = booke206_get_tlbm(env, i, address, j);
            if (!tlb) {
                continue;
            }
            ret = mmubooke206_check_tlb(env, tlb, &raddr, &ctx->prot, address,
                                        rw, access_type, 0);
            if (ret != -1) {
                goto found_tlb;
            }
        }
    }

found_tlb:

    if (ret >= 0) {
        ctx->raddr = raddr;

        *write_through_enabled = (tlb->mas2 >> MAS2_W_SHIFT) & 0x1;
        *coherency_enabled = (tlb->mas2 >> MAS2_M_SHIFT) & 0x1;

        /* For cache inhibition enabled we check the cache ennabled bit
         * Qemu has a hard time detecting the data/inst access with the
         * access_type variable
         */
        if(real_access_type == ACCESS_CODE)
        {
            /* Get the register */
            if((env->spr[SPR_Exxx_L1CSR1] & L1CSR1_ICE) == 0)
                *cache_inhibit = 1;
            else
                *cache_inhibit = (tlb->mas2 >> MAS2_I_SHIFT) & 0x1;
        }
        else
        {
            /* Get the register */
            if((env->spr[SPR_Exxx_L1CSR0] & L1CSR0_DCE) == 0)
                *cache_inhibit = 1;
            else
                *cache_inhibit = (tlb->mas2 >> MAS2_I_SHIFT) & 0x1;
        }

    }

    return ret;
}

static int ppc_get_info_addr_mem_trace_internal(CPUPPCState *env, mmu_ctx_t *ctx,
                                   target_ulong eaddr, int rw,
                                   int access_type,
                                   int real_access_type,
                                   int32_t* write_through_enabled,
                                   int32_t* cache_inhibit,
                                   int32_t* coherency_enabled)
{
    PowerPCCPU *cpu = ppc_env_get_cpu(env);
    int ret = -1;
    bool real_mode = (access_type == ACCESS_CODE && msr_ir == 0)
        || (access_type != ACCESS_CODE && msr_dr == 0);


    switch (env->mmu_model) {
    case POWERPC_MMU_SOFT_6xx:
    case POWERPC_MMU_SOFT_74xx:
        if (real_mode) {
            ret = check_physical(env, ctx, eaddr, rw);
        } else {
            /* Try to find a BAT */
            if (env->nb_BATs != 0) {
                ret = get_bat_6xx_tlb(env, ctx, eaddr, rw, access_type);
            }
            if (ret < 0) {
                /* We didn't match any BAT entry or don't have BATs */
                ret = get_segment_6xx_tlb(env, ctx, eaddr, rw, access_type);
            }
        }
        break;

    case POWERPC_MMU_SOFT_4xx:
    case POWERPC_MMU_SOFT_4xx_Z:
        if (real_mode) {
            ret = check_physical(env, ctx, eaddr, rw);
        } else {
            ret = mmu40x_get_physical_address(env, ctx, eaddr,
                                              rw, access_type);
        }
        break;
    case POWERPC_MMU_BOOKE:
        ret = mmubooke_get_physical_address(env, ctx, eaddr,
                                            rw, access_type);
        break;
    case POWERPC_MMU_BOOKE206:
        ret = mmubooke206_get_info(env, ctx, eaddr, rw,
                                access_type, real_access_type,
                                write_through_enabled, cache_inhibit,
                                coherency_enabled);

        break;
    case POWERPC_MMU_MPC8xx:
        /* XXX: TODO */
        cpu_abort(CPU(cpu), "MPC8xx MMU model is not implemented\n");
        break;
    case POWERPC_MMU_REAL:
        if (real_mode) {
            ret = check_physical(env, ctx, eaddr, rw);
        } else {
            cpu_abort(CPU(cpu), "PowerPC in real mode do not do any translation\n");
        }
        return -1;
    default:
        cpu_abort(CPU(cpu), "Unknown or invalid MMU model\n");
        return -1;
    }
#if 0
    qemu_log("%s address " TARGET_FMT_lx " => %d " TARGET_FMT_plx "\n",
             __func__, eaddr, ret, ctx->raddr);
#endif
    return ret;
}

void ppc_get_info_addr_mem_trace(CPUPPCState *env, vaddr addr,
                                   int real_access_type,
                                   target_ulong* phys_addr,
                                   int32_t* write_through_enabled,
                                   int32_t* cache_inhibit,
                                   int32_t* coherency_enabled)
{
    mmu_ctx_t ctx;
    *cache_inhibit = -1;
    *coherency_enabled = -1;
    *phys_addr = -1;

    if (unlikely(ppc_get_info_addr_mem_trace_internal(env, &ctx, addr, 0,
        ACCESS_INT, real_access_type, write_through_enabled,
        cache_inhibit, coherency_enabled) != 0)) {

        /* Some MMUs have separate TLBs for code and data. If we only try an
         * ACCESS_INT, we may not be able to read instructions mapped by code
         * TLBs, so we also try a ACCESS_CODE.
         */
        if (unlikely(ppc_get_info_addr_mem_trace_internal(env, &ctx, addr, 0,
                                  ACCESS_CODE, real_access_type,
                                  write_through_enabled,
                                  cache_inhibit, coherency_enabled) != 0)) {
            return;
        }
    }
#if QEM_TRACE_PHYSICAL_ADDRESS
    *phys_addr = ctx.raddr;
#else 
    *phys_addr = addr;
#endif
}


#endif /* QEM_TRACE_ENABLED */