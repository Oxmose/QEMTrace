/*
 * Guest memory access trace definition
 *
 * Provides QEMTrace the format of the trace.
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

#ifndef __QEM_TRACE_DEF_H_
#define __QEM_TRACE_DEF_H_

#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

/*******************************************************************************
 * STRUCTURES AND MASKS
 ******************************************************************************/

/* Tracing types */
typedef enum 
{
    QEM_TRACE_DTRACE    = 0x01,
    QEM_TRACE_ITRACE    = 0x02,

    /* MUST be equal to (QEM_TRACE_DTRACE | QEM_TRACE_ITRACE) */
    QEM_TRACE_ALLTRACE  = 0x03 
} QEM_TRACE_TTYPE_E;

#if QEM_TRACE_GATHER_META

/* Tracing acces types, if the access is a read or a write */
typedef enum
{
    QEM_TRACE_ACCESS_TYPE_READ  = 0x00000000,
    QEM_TRACE_ACCESS_TYPE_WRITE = 0x00000100
} QEM_TRACE_ACCESS_TYPE_E;
#define QEM_TRACE_ACCESS_TYPE_MASK 0x00000100

/* Access source, instruction of data */
typedef enum
{
    QEM_TRACE_DATA_TYPE_DATA = 0x00000000,
    QEM_TRACE_DATA_TYPE_INST = 0x00000200
} QEM_TRACE_DATA_TYPE_E;
#define QEM_TRACE_DATA_TYPE_MASK 0x00000200

/* Current CPU protection level */
typedef enum
{
    QEM_TRACE_PL_KERNEL = 0x00000000,
    QEM_TRACE_PL_USER   = 0x00000400
} QEM_TRACE_PL_E;
#define QEM_TRACE_PL_E_MASK 0x00000400

/* Memory event type */
typedef enum
{
    QEM_TRACE_EVENT_ACCESS     = 0x00000000,
    QEM_TRACE_EVENT_FLUSH      = 0x00000800,
    QEM_TRACE_EVENT_INVALIDATE = 0x00001000,
    /* FLUSH_INVAL = QEM_TRACE_EVENT_FLUSH | QEM_TRACE_EVENT_INVALIDATE */
    QEM_TRACE_EVENT_LOCK       = 0x00002000,
    QEM_TRACE_EVENT_UNLOCK     = 0x00002800,
    QEM_TRACE_EVENT_PREFETCH   = 0x00003000,
    QEM_TRACE_EVENT_DCBZ       = 0x00003800
} QEM_TRACE_EVENT_E;
#define QEM_TRACE_EVENT_MASK 0x00003800

/* Request granularity, NONE for non request type traces like accesses */
typedef enum
{
    QEM_TRACE_GRANULARITY_LINE   = 0x00000000,
    QEM_TRACE_GRANULARITY_SET    = 0x00004000,
    QEM_TRACE_GRANULARITY_WAY    = 0x00008000,
    QEM_TRACE_GRANULARITY_GLOBAL = 0x0000C000,
    QEM_TRACE_GRANULARITY_NONE   = 0x00000000
} QEM_TRACE_GRANULARITY_E;
#define QEM_TRACE_GRANULARITY_MASK 0x0000C000

/* Access size */
typedef enum
{
    QEM_TRACE_DATA_SIZE_8_BITS  = 0x00000000,
    QEM_TRACE_DATA_SIZE_16_BITS = 0x00010000,
    QEM_TRACE_DATA_SIZE_32_BITS = 0x00020000,
    QEM_TRACE_DATA_SIZE_64_BITS = 0x00030000
} QEM_TRACE_DATA_SIZE_E;
#define QEM_TRACE_DATA_SIZE_MASK 0x00030000

/* Coherency required */
typedef enum
{
    QEM_TRACE_COHERENCY_NON_REQ = 0x00000000,
    QEM_TRACE_COHERENCY_REQ     = 0x00040000
} QEM_TRACE_COHERENCY_E;
#define QEM_TRACE_COHERENCY_MASK 0x00040000

/* Cache level */
typedef enum
{
    QEM_TRACE_CACHE_LEVEL_1   = 0x00000000,
    QEM_TRACE_CACHE_LEVEL_2   = 0x00080000,
    QEM_TRACE_CACHE_LEVEL_3   = 0x00100000,
    QEM_TRACE_CACHE_LEVEL_ALL = 0x00180000
} QEM_TRACE_CACHE_LEVEL_E;
#define QEM_TRACE_CACHE_LEVEL_MASK 0x00180000

/* Cache inhibited state */
typedef enum
{
    QEM_TRACE_CACHE_INHIBIT_OFF = 0x00000000,
    QEM_TRACE_CACHE_INHIBIT_ON  = 0x00200000,
} QEM_TRACE_CACHE_INHIBIT_E;
#define QEM_TRACE_CACHE_INHIBIT_MASK 0x00200000

/* Write Through enable state */
typedef enum
{
    QEM_TRACE_CACHE_WT_OFF = 0x00000000,
    QEM_TRACE_CACHE_WT_ON  = 0x00400000
} QEM_TRACE_CACHE_WT_E;
#define QEM_TRACE_CACHE_WT_MASK 0x00400000

/* Exclusive event state, this is used to notify the state of a cache line
 * should be exclusive at the end of the instruction execution under certain
 * conditions
 */
typedef enum
{
    QEM_TRACE_EVENT_NON_EXCLUSIVE = 0x00000000,
    QEM_TRACE_EVENT_EXCLUSIVE     = 0x00800000
} QEM_TRACE_EVENT_EXCLUSIVE_E;
#define QEM_TRACE_EVENT_EXCLUSIVE_MASK 0x00800000

#else 
typedef enum
{
    QEM_TRACE_ACCESS_TYPE_READ  = 0x00000000,
    QEM_TRACE_ACCESS_TYPE_WRITE = 0x00000001
} QEM_TRACE_ACCESS_TYPE_E;
#define QEM_TRACE_ACCESS_TYPE_MASK 0x00000001

/* Access source, instruction of data */
typedef enum
{
    QEM_TRACE_DATA_TYPE_DATA = 0x00000000,
    QEM_TRACE_DATA_TYPE_INST = 0x00000002
} QEM_TRACE_DATA_TYPE_E;
#define QEM_TRACE_DATA_TYPE_MASK 0x00000002

/* Memory event type */
typedef enum
{
    QEM_TRACE_EVENT_ACCESS     = 0x00000000,
    QEM_TRACE_EVENT_FLUSH      = 0x00000004,
    QEM_TRACE_EVENT_INVALIDATE = 0x00000008,
    /* FLUSH_INVAL = QEM_TRACE_EVENT_FLUSH | QEM_TRACE_EVENT_INVALIDATE */
    QEM_TRACE_EVENT_LOCK       = 0x0000000C,
    QEM_TRACE_EVENT_UNLOCK     = 0x00000010,
    QEM_TRACE_EVENT_PREFETCH   = 0x00000014,
    QEM_TRACE_EVENT_DCBZ       = 0x00000018
} QEM_TRACE_EVENT_E;
#define QEM_TRACE_EVENT_MASK 0x0000001C

/* Current CPU protection level */
typedef enum
{
    QEM_TRACE_PL_KERNEL = 0x00000000,
    QEM_TRACE_PL_USER   = 0x00000000
} QEM_TRACE_PL_E;
#define QEM_TRACE_PL_E_MASK 0x00000000

/* Request granularity, NONE for non request type traces like accesses */
typedef enum
{
    QEM_TRACE_GRANULARITY_LINE   = 0x00000000,
    QEM_TRACE_GRANULARITY_SET    = 0x00000000,
    QEM_TRACE_GRANULARITY_WAY    = 0x00000000,
    QEM_TRACE_GRANULARITY_GLOBAL = 0x00000000,
    QEM_TRACE_GRANULARITY_NONE   = 0x00000000
} QEM_TRACE_GRANULARITY_E;
#define QEM_TRACE_GRANULARITY_MASK 0x00000000

/* Access size */
typedef enum
{
    QEM_TRACE_DATA_SIZE_8_BITS  = 0x00000000,
    QEM_TRACE_DATA_SIZE_16_BITS = 0x00000000,
    QEM_TRACE_DATA_SIZE_32_BITS = 0x00000000,
    QEM_TRACE_DATA_SIZE_64_BITS = 0x00000000
} QEM_TRACE_DATA_SIZE_E;
#define QEM_TRACE_DATA_SIZE_MASK 0x00000000

/* Coherency required */
typedef enum
{
    QEM_TRACE_COHERENCY_NON_REQ = 0x00000000,
    QEM_TRACE_COHERENCY_REQ     = 0x00000000
} QEM_TRACE_COHERENCY_E;
#define QEM_TRACE_COHERENCY_MASK 0x00000000

/* Cache level */
typedef enum
{
    QEM_TRACE_CACHE_LEVEL_1   = 0x00000000,
    QEM_TRACE_CACHE_LEVEL_2   = 0x00000000,
    QEM_TRACE_CACHE_LEVEL_3   = 0x00000000,
    QEM_TRACE_CACHE_LEVEL_ALL = 0x00000000
} QEM_TRACE_CACHE_LEVEL_E;
#define QEM_TRACE_CACHE_LEVEL_MASK 0x00000000

/* Cache inhibited state */
typedef enum
{
    QEM_TRACE_CACHE_INHIBIT_OFF = 0x00000000,
    QEM_TRACE_CACHE_INHIBIT_ON  = 0x00000000,
} QEM_TRACE_CACHE_INHIBIT_E;
#define QEM_TRACE_CACHE_INHIBIT_MASK 0x00000000

/* Write Through enable state */
typedef enum
{
    QEM_TRACE_CACHE_WT_OFF = 0x00000000,
    QEM_TRACE_CACHE_WT_ON  = 0x00000000
} QEM_TRACE_CACHE_WT_E;
#define QEM_TRACE_CACHE_WT_MASK 0x00000000

/* Exclusive event state, this is used to notify the state of a cache line
 * should be exclusive at the end of the instruction execution under certain
 * conditions
 */
typedef enum
{
    QEM_TRACE_EVENT_NON_EXCLUSIVE = 0x00000000,
    QEM_TRACE_EVENT_EXCLUSIVE     = 0x00000000
} QEM_TRACE_EVENT_EXCLUSIVE_E;
#define QEM_TRACE_EVENT_EXCLUSIVE_MASK 0x00000000

#endif /* QEM_TRACE_GATHER_META */

#endif /* QEM_TRACE_ENABLED */

#endif /* __QEM_TRACE_DEF_H_ */