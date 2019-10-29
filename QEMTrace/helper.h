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

#include "qem_trace_config.h" /* QEMTrace configuration */

#if QEM_TRACE_ENABLED

#if defined(TARGET_PPC)
#include "arch/ppc/helper.h"
#endif

#if defined(TARGET_X86_64) || defined(TARGET_I386)
#include "arch/i386/helper.h"
#endif
#endif /* QEM_TRACE_ENABLED */