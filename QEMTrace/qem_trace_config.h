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
#define QEM_TRACE_START_TIMER_OP    0xFFFFFFF2
#define QEM_TRACE_GET_TIMER_OP      0xFFFFFFF3
#define QEM_TRACE_FLASH_INV_INST_OP 0xFFFFFFF4
#define QEM_TRACE_FLASH_INV_DATA_OP 0xFFFFFFF5
    
/******************************
 * Trace type 
 *****************************/

/* Print trace to stdio (1) or output file(0) */
#define QEM_TRACE_PRINT 1

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

/* TODO add data, instruction tracing config */

/******************************
 * Trace buffers 
 *****************************/

/* Size of the write buffer in number of entries */
#define QEM_TRACE_BUFFER_SIZE 1024

/* Stream buffer size in bytes */
#define QEM_TRACE_STREAM_BUFFER_SIZE 65536

#endif /* __QEM_TRACE_CONFIG_H_ */