#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

typedef struct mem_trace_32
{
    uint32_t address;
    uint64_t timestamp;
    uint32_t  flags;
}__attribute__((packed)) mem_trace_32_t;

typedef struct mem_trace_64
{
    uint64_t address;
    uint64_t timestamp;
    uint32_t  flags;
}__attribute__((packed)) mem_trace_64_t;

typedef struct mem_trace_header
{
    uint64_t size;
    uint16_t struct_size;
    uint8_t  version;
    uint8_t  reserved[5];
}__attribute__((packed)) mem_trace_header_t;

int main(int argc, char** argv)
{
    int size = -1;
    if(argc != 3)
    {
        printf("ERROR, wrong usage\n");
        return -1;
    }
    FILE* fdin = fopen(argv[1], "rb");
    if(fdin < 0)
    {
        printf("Cannot open input file\n");
        return -1;
    }

    mem_trace_32_t w_buf;
    mem_trace_64_t l_buf;

    void* buffer;

    if(strncmp(argv[2], "32", 2) == 0)
    {
        size = sizeof(mem_trace_32_t);
        buffer = &w_buf;

    }
    else if(strncmp(argv[2], "64", 2) == 0)
    {
        size = sizeof(mem_trace_64_t);
        buffer = &l_buf;
    }

    if(size == -1)
    {
        printf("Error, wrong bit format\n");
        return -1;
    }

    /* read header */
    mem_trace_header_t header;
    fread(&header, sizeof(mem_trace_header_t), 1, fdin);
    //printf("Trace file size %lu, trace size: %d, version %d\n",
    //        header.size, header.struct_size, header.version);

    while(fread(buffer, size, 1, fdin) == 1)
    {
        if(size == sizeof(mem_trace_32_t))
        {
            ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_DCBZ) ? printf("D ") : (
                ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_PREFETCH) ? printf("P ") : (
                    ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_UNLOCK) ? printf("U ") : (
                         ((w_buf.flags & MEM_TRACE_EVENT_LOCK) == MEM_TRACE_EVENT_LOCK) ? printf("L ") : (
                                 ((w_buf.flags & MEM_TRACE_EVENT_MASK) == (MEM_TRACE_EVENT_INVALIDATE | MEM_TRACE_EVENT_FLUSH)) ? printf("FL/INV ") : (
                                     ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_INVALIDATE) ? printf("INV ") : (
                                         ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_FLUSH) ? printf("FL ") : ((w_buf.flags & MEM_TRACE_DATA_TYPE_INST) ? printf("I "): printf("D "))))))));


            if(w_buf.flags & MEM_TRACE_ACCESS_TYPE_WRITE)
            {
                printf("ST ");
            }
            else
            {
                printf("LD ");
            }

            printf("| P 0x%08x | Core %d | Time %llu | RW: %c | ID: %c | PL: %c | E: %c | G: %c | S: %c | CR: %c | CL: %c | CI: %c | WT: %c | EX: %c\n",
                    w_buf.address, w_buf.flags & 0xFF, w_buf.timestamp,
                   (w_buf.flags & MEM_TRACE_ACCESS_TYPE_WRITE) ? 'W' : 'R',

                   (w_buf.flags & MEM_TRACE_DATA_TYPE_INST) ? 'I' : 'D',

                   (w_buf.flags & MEM_TRACE_PL_USER) ? 'U' : 'K',

                   ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_DCBZ) ? 'D' : (
                       ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_PREFETCH) ? 'P' : (
                           ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_UNLOCK) ? 'U' : (
                                ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_LOCK) ? 'L' : (
                                        ((w_buf.flags & MEM_TRACE_EVENT_MASK) == (MEM_TRACE_EVENT_INVALIDATE | MEM_TRACE_EVENT_FLUSH)) ? 'B' : (
                                            ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_INVALIDATE) ? 'I' : (
                                                ((w_buf.flags & MEM_TRACE_EVENT_MASK) == MEM_TRACE_EVENT_FLUSH) ? 'F' : 'A')))))),

                   ((w_buf.flags & MEM_TRACE_GRANULARITY_MASK) == MEM_TRACE_GRANULARITY_SET) ? 'S' :
                   ((w_buf.flags & MEM_TRACE_GRANULARITY_MASK) == MEM_TRACE_GRANULARITY_WAY) ? 'W' :
                   ((w_buf.flags & MEM_TRACE_GRANULARITY_MASK) == MEM_TRACE_GRANULARITY_GLOBAL) ? 'G' : 'L',

                   ((w_buf.flags & MEM_TRACE_DATA_SIZE_MASK) == MEM_TRACE_DATA_SIZE_16_BITS) ? 'H' :
                   ((w_buf.flags & MEM_TRACE_DATA_SIZE_MASK) == MEM_TRACE_DATA_SIZE_32_BITS) ? 'W' :
                   ((w_buf.flags & MEM_TRACE_DATA_SIZE_MASK) == MEM_TRACE_DATA_SIZE_64_BITS) ? 'D' : 'B',

                   (w_buf.flags & MEM_TRACE_COHERENCY_REQ) ? 'Y' : 'N',

                   ((w_buf.flags & MEM_TRACE_CACHE_LEVEL_MASK) == MEM_TRACE_CACHE_LEVEL_ALL) ? 'A' :
                   ((w_buf.flags & MEM_TRACE_CACHE_LEVEL_MASK) == MEM_TRACE_CACHE_LEVEL_3) ? '3' :
                   ((w_buf.flags & MEM_TRACE_CACHE_LEVEL_MASK) == MEM_TRACE_CACHE_LEVEL_2) ? '2' : '1',

                   (w_buf.flags & MEM_TRACE_CACHE_INHIBIT_ON) ? 'Y' : 'N',

                   (w_buf.flags & MEM_TRACE_CACHE_WT_ON) ? 'E' : 'D',

                   (w_buf.flags & MEM_TRACE_EVENT_EXCLUSIVE) ? 'Y' : 'N'
                );
        }

        else
        {
            ((l_buf.flags & MEM_TRACE_EVENT_DCBZ) == MEM_TRACE_EVENT_DCBZ) ? printf("D ") : (
                ((l_buf.flags & MEM_TRACE_EVENT_PREFETCH) == MEM_TRACE_EVENT_PREFETCH) ? printf("P ") : (
                    ((l_buf.flags & MEM_TRACE_EVENT_UNLOCK) == MEM_TRACE_EVENT_UNLOCK) ? printf("U ") : (
                         ((l_buf.flags & MEM_TRACE_EVENT_LOCK) == MEM_TRACE_EVENT_LOCK) ? printf("L ") : (
                                 ((l_buf.flags & (MEM_TRACE_EVENT_INVALIDATE | MEM_TRACE_EVENT_FLUSH)) == (MEM_TRACE_EVENT_INVALIDATE | MEM_TRACE_EVENT_FLUSH)) ? printf("FL/INV ") : (
                                     ((l_buf.flags & MEM_TRACE_EVENT_INVALIDATE) == MEM_TRACE_EVENT_INVALIDATE) ? printf("INV ") : (
                                         ((l_buf.flags & MEM_TRACE_EVENT_FLUSH) == MEM_TRACE_EVENT_FLUSH) ? printf("FL ") : ((l_buf.flags & MEM_TRACE_DATA_TYPE_INST) ? printf("I "): printf("D "))))))));


            if(l_buf.flags & MEM_TRACE_ACCESS_TYPE_WRITE)
            {
                printf("ST ");
            }
            else
            {
                printf("LD ");
            }

            printf("| P 0x%16x | Core %d | Time %llu | RW: %c | ID: %c | PL: %c | E: %c | G: %c | S: %c | CR: %c | CL: %c | CI: %c | WT: %c | EX: %c\n",
                    w_buf.address, w_buf.flags & 0xFF, w_buf.timestamp,
                   (l_buf.flags & MEM_TRACE_ACCESS_TYPE_WRITE) ? 'W' : 'R',

                   (l_buf.flags & MEM_TRACE_DATA_TYPE_INST) ? 'I' : 'D',

                   (l_buf.flags & MEM_TRACE_PL_USER) ? 'U' : 'K',

                   ((l_buf.flags & MEM_TRACE_EVENT_DCBZ) == MEM_TRACE_EVENT_DCBZ) ? 'D' : (
                       ((l_buf.flags & MEM_TRACE_EVENT_PREFETCH) == MEM_TRACE_EVENT_PREFETCH) ? 'P' : (
                           ((l_buf.flags & MEM_TRACE_EVENT_UNLOCK) == MEM_TRACE_EVENT_UNLOCK) ? 'U' : (
                                ((l_buf.flags & MEM_TRACE_EVENT_LOCK) == MEM_TRACE_EVENT_LOCK) ? 'L' : (
                                        ((l_buf.flags & (MEM_TRACE_EVENT_INVALIDATE | MEM_TRACE_EVENT_FLUSH)) == (MEM_TRACE_EVENT_INVALIDATE | MEM_TRACE_EVENT_FLUSH)) ? 'B' : (
                                            ((l_buf.flags & MEM_TRACE_EVENT_INVALIDATE) == MEM_TRACE_EVENT_INVALIDATE) ? 'I' : (
                                                ((l_buf.flags & MEM_TRACE_EVENT_FLUSH) == MEM_TRACE_EVENT_FLUSH) ? 'F' : 'A')))))),

                   ((l_buf.flags & MEM_TRACE_GRANULARITY_SET) == MEM_TRACE_GRANULARITY_SET) ? 'S' :
                   ((l_buf.flags & MEM_TRACE_GRANULARITY_WAY) == MEM_TRACE_GRANULARITY_WAY) ? 'W' :
                   ((l_buf.flags & MEM_TRACE_GRANULARITY_GLOBAL) == MEM_TRACE_GRANULARITY_GLOBAL) ? 'G' : 'L',

                   ((l_buf.flags & MEM_TRACE_DATA_SIZE_16_BITS) == MEM_TRACE_DATA_SIZE_16_BITS) ? 'H' :
                   ((l_buf.flags & MEM_TRACE_DATA_SIZE_32_BITS) == MEM_TRACE_DATA_SIZE_32_BITS) ? 'W' :
                   ((l_buf.flags & MEM_TRACE_DATA_SIZE_64_BITS) == MEM_TRACE_DATA_SIZE_64_BITS) ? 'D' : 'B',

                   (l_buf.flags & MEM_TRACE_COHERENCY_REQ) ? 'Y' : 'N',

                   ((l_buf.flags & MEM_TRACE_CACHE_LEVEL_ALL) == MEM_TRACE_CACHE_LEVEL_ALL) ? 'A' :
                   ((l_buf.flags & MEM_TRACE_CACHE_LEVEL_3) == MEM_TRACE_CACHE_LEVEL_3) ? '3' :
                   ((l_buf.flags & MEM_TRACE_CACHE_LEVEL_2) == MEM_TRACE_CACHE_LEVEL_2) ? '2' : '1',

                   (l_buf.flags & MEM_TRACE_CACHE_INHIBIT_ON) ? 'Y' : 'N',

                   (l_buf.flags & MEM_TRACE_CACHE_WT_ON) ? 'E' : 'D',

                   (l_buf.flags & MEM_TRACE_EVENT_EXCLUSIVE) ? 'Y' : 'N'
               );
        }
    }
    return 0;
}
