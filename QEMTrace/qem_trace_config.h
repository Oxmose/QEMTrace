/*
 * Guest memory access tracing configuration
 *
 * Provides QEMTrace the configuration needed to compile Qemu with the 
 * proper set of tools.
 * 
 * Header included in
 *     qem_trace_engine.h
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

#ifndef __QEM_TRACE_CONFIG_H_
#define __QEM_TRACE_CONFIG_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* Enable QEM TRACE */
#define QEM_TRACE_ENABLED 1

/******************************
 * Trace instructions 
 *****************************/
#define QEM_TRACE_START_OP          0xFFFFFFF0
#define QEM_TRACE_STOP_OP           0xFFFFFFF1
#define QEM_TRACE_DSTART_OP         0xFFFFFFF2
#define QEM_TRACE_DSTOP_OP          0xFFFFFFF3
#define QEM_TRACE_ISTART_OP         0xFFFFFFF4
#define QEM_TRACE_ISTOP_OP          0xFFFFFFF5

#define QEM_TRACE_START_TIMER_OP    0xFFFFFFF6
#define QEM_TRACE_GET_TIMER_OP      0xFFFFFFF7

#define QEM_TRACE_FLASH_INV_INST_OP 0xFFFFFFF8
#define QEM_TRACE_FLASH_INV_DATA_OP 0xFFFFFFF9
    
/******************************
 * Trace type 
 *****************************/
#define QEM_TRACE_PRINT 0
#define QEM_TRACE_FILE  1
#define QEM_TRACE_SMI   2

/* Select the trace type */
#define QEM_TRACE_TYPE QEM_TRACE_FILE

/* Modify this value depending on the system you want to trace
 * 0 = 32 Bits target
 * 1 = 64 Bits target
 */
#define QEM_TRACE_TARGET_64 0

/******************************
 * Trace configuration 
 *****************************/

/* Modify this value depending on the type of address you want:
 * 1 = Physical address
 * 0 = Virtual address
 */
#define QEM_TRACE_PHYSICAL_ADDRESS 1

/* Set this value to 0 if you do not want metadata to be gathered during tracing.
 * Only the trace type will be stored.
 * 1 = Metadata gathering enabled
 * 0 = Metadata gathering disabled
 */
#define QEM_TRACE_GATHER_META 1

/******************************
 * Trace buffers 
 *****************************/

/* Size of the trace file buffer in number of entries */
#define QEM_TRACE_BUFFER_SIZE 10000

/* Define the multi threaded buffer state */
#define QEM_TRACE_MT_BUFFER_EN 1

/* SMI Settings */
#define QEM_SMI_BLOCK_COUNT 8
#define QEM_SMI_BLOCK_SIZE  10000000

#endif /* __QEM_TRACE_CONFIG_H_ */