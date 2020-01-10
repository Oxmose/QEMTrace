/*
 * SMI Interface test client
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

#include <stdio.h>
#include <time.h>
#include "../../SMILib/include/qem_posix_smi.h"


typedef struct qem_trace
{
    uint32_t address;
    uint64_t timestamp;
    uint32_t  flags;
}__attribute__((packed)) qem_trace_t;

//* Tracing acces types, if the access is a read or a write */
typedef enum
{
    MEM_TRACE_ACCESS_TYPE_READ  = 0x00000000,
    MEM_TRACE_ACCESS_TYPE_WRITE = 0x00000100
} MEM_TRACE_ACCESS_TYPE_E;
#define MEM_TRACE_ACCESS_TYPE_MASK 0x00000100

/* Access source, instruction of data */
typedef enum
{
    MEM_TRACE_DATA_TYPE_DATA = 0x00000000,
    MEM_TRACE_DATA_TYPE_INST = 0x00000200
} MEM_TRACE_DATA_TYPE_E;
#define MEM_TRACE_DATA_TYPE_MASK 0x00000200

/* Current CPU protection level */
typedef enum
{
    MEM_TRACE_PL_KERNEL = 0x00000000,
    MEM_TRACE_PL_USER   = 0x00000400
} MEM_TRACE_PL_E;
#define MEM_TRACE_PL_E_MASK 0x00000400

/* Memory event type */
typedef enum
{
    MEM_TRACE_EVENT_ACCESS     = 0x00000000,
    MEM_TRACE_EVENT_FLUSH      = 0x00000800,
    MEM_TRACE_EVENT_INVALIDATE = 0x00001000,
    /* FLUSH_INVAL = MEM_TRACE_EVENT_FLUSH | MEM_TRACE_EVENT_INVALIDATE */
    MEM_TRACE_EVENT_LOCK       = 0x00002000,
    MEM_TRACE_EVENT_UNLOCK     = 0x00002800,
    MEM_TRACE_EVENT_PREFETCH   = 0x00003000,
    MEM_TRACE_EVENT_DCBZ       = 0x00003800
} MEM_TRACE_EVENT_E;
#define MEM_TRACE_EVENT_MASK 0x00003800

/* Request granularity, NONE for non request type traces like accesses */
typedef enum
{
    MEM_TRACE_GRANULARITY_LINE   = 0x00000000,
    MEM_TRACE_GRANULARITY_SET    = 0x00004000,
    MEM_TRACE_GRANULARITY_WAY    = 0x00008000,
    MEM_TRACE_GRANULARITY_GLOBAL = 0x0000C000,
    MEM_TRACE_GRANULARITY_NONE   = 0x00000000
} MEM_TRACE_GRANULARITY_E;
#define MEM_TRACE_GRANULARITY_MASK 0x0000C000

/* Access size */
typedef enum
{
    MEM_TRACE_DATA_SIZE_8_BITS  = 0x00000000,
    MEM_TRACE_DATA_SIZE_16_BITS = 0x00010000,
    MEM_TRACE_DATA_SIZE_32_BITS = 0x00020000,
    MEM_TRACE_DATA_SIZE_64_BITS = 0x00030000
} MEM_TRACE_DATA_SIZE_E;
#define MEM_TRACE_DATA_SIZE_MASK 0x00030000

/* Coherency required */
typedef enum
{
    MEM_TRACE_COHERENCY_NON_REQ = 0x00000000,
    MEM_TRACE_COHERENCY_REQ     = 0x00040000
} MEM_TRACE_COHERENCY_E;
#define MEM_TRACE_COHERENCY_MASK 0x00040000

/* Cache level */
typedef enum
{
    MEM_TRACE_CACHE_LEVEL_1   = 0x00000000,
    MEM_TRACE_CACHE_LEVEL_2   = 0x00080000,
    MEM_TRACE_CACHE_LEVEL_3   = 0x00100000,
    MEM_TRACE_CACHE_LEVEL_ALL = 0x00180000
} MEM_TRACE_CACHE_LEVEL_E;
#define MEM_TRACE_CACHE_LEVEL_MASK 0x00180000

/* Cache inhibited state */
typedef enum
{
    MEM_TRACE_CACHE_INHIBIT_OFF = 0x00000000,
    MEM_TRACE_CACHE_INHIBIT_ON  = 0x00200000,
} MEM_TRACE_CACHE_INHIBIT_E;
#define MEM_TRACE_CACHE_INHIBIT_MASK 0x00200000

/* Write Through enable state */
typedef enum
{
    MEM_TRACE_CACHE_WT_OFF = 0x00000000,
    MEM_TRACE_CACHE_WT_ON  = 0x00400000
} MEM_TRACE_CACHE_WT_E;
#define MEM_TRACE_CACHE_WT_MASK 0x00400000

/* Exclusive event state, this is used to notify the state of a cache line
 * should be exclusive at the end of the instruction execution under certain
 * conditions
 */
typedef enum
{
    MEM_TRACE_EVENT_NON_EXCLUSIVE = 0x00000000,
    MEM_TRACE_EVENT_EXCLUSIVE     = 0x00800000
} MEM_TRACE_EVENT_EXCLUSIVE_E;
#define MEM_TRACE_EVENT_EXCLUSIVE_MASK 0x00800000

int main(int argc, char** argv) 
{
    (void)argc;
    (void)argv;

    qem_trace_t w_buf;
    uint32_t val;

    /* Connect */
    qem_smi_init();
    qem_smi_connect(2, 5);

    struct timespec tstart={0,0}, tend={0,0};
    
    clock_gettime(CLOCK_MONOTONIC, &tstart);
   

    printf("Connected\n");
    while(1)
    {
        qem_smi_receive((void*)&val, sizeof(uint32_t));
        if(val != QEM_SMI_STREAM_VERSION)
        {
            printf("Wrong stream version");
            return -1;
        }
        qem_smi_receive((void*)&val, sizeof(uint32_t));
        if(val != sizeof(qem_trace_t))
        {
            printf("Wrong trace size");
            return -1;
        }

        /* Get traces until end of stream */
        qem_smi_receive((void*)&w_buf, sizeof(qem_trace_t));
        while(w_buf.address != 0x0 || w_buf.timestamp != 0x0)
        {      
            qem_smi_receive((void*)&w_buf, sizeof(qem_trace_t));
        }
        qem_smi_post_server_flush();
        fflush(stdout);
        break;
    }

    clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("some_long_computation took about %.5f seconds\n",
           ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
    
    return 0;
}
