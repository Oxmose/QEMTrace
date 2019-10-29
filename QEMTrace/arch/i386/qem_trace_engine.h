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

#include "sysemu/kvm.h"

#include "qemu/error-report.h"



#include "../../qem_trace_config.h" /* QEMTrace configuration */

#if QEM_TRACE_ENABLED
#include "../../qem_trace_engine.h"

target_ulong qem_get_phys_addr_mem_tracing(CPUArchState* env,
                                       uint32_t* cache_inhibit,
                                       const target_ulong addr);

uint32_t qem_get_flags_mem_tracing(const CPUArchState* env,
                               const QEM_TRACE_EVENT_E trace_type,
                               const QEM_TRACE_ACCESS_TYPE_E access_type,
                               const QEM_TRACE_DATA_SIZE_E data_type,
                               const QEM_TRACE_GRANULARITY_E granularity,
                               const uint32_t data_size,
                               const target_ulong phys_addr);



#endif /* QEM_TRACE_ENABLED */