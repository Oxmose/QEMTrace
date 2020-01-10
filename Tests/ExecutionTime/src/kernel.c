/*******************************************************************************
 *
 * File: kernel.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/06/2018
 *
 * Version: 1.0
 *
 * Boot code for the PowerPC e500 version of MiniKernel
 ******************************************************************************/

#include "tlb.h"          
#include "mmu.h"         
#include "printf.h"
#include "common.h"

/* Test entry point */

extern unsigned int kernel_offset;
extern unsigned int kernel_stack_start;
extern unsigned int kernel_stack_base;

extern unsigned int free_mem;

unsigned int last_entry_tlb1 = 0;
unsigned int last_entry_tlb0 = 0;

extern int main(void);

static void bsp_init(void)
{
    unsigned int i;

    /* First entry mapp the current kernel address space */
    last_entry_tlb1 = 1;

    /* Map CCSRBAR */
    tlb_set_entry(1, last_entry_tlb1++, 1, 0, 0, BOOKE_PAGESZ_1M, CCSBAR_BASE_ADDR,
                  MAS2_I | MAS2_G | MAS2_W, CCSBAR_BASE_PHYS_ADDR, MAS3_SW | MAS3_SR);

    /* Map kernel code + data */
    tlb_set_entry(1, last_entry_tlb1++, 1, 0, 0, BOOKE_PAGESZ_64K, 
        (unsigned int)&kernel_offset, 
        MAS2_G , 
        (unsigned int)&kernel_offset,
        MAS3_SX | MAS3_SW | MAS3_SR);

    /* Map kernel stack */
    tlb_set_entry(1, last_entry_tlb1++, 1, 0, 0, BOOKE_PAGESZ_16K, 
        (unsigned int)&kernel_stack_start, 
        MAS2_G, 
        (unsigned int)&kernel_stack_start,
        MAS3_SW | MAS3_SR);

    /* Unmap all other entries */
    for(i = last_entry_tlb1;  i < 16; ++i)
    {
        tlb_rem_entry(1, i);
    }
    /* Unmap first entry */
    tlb_rem_entry(1, 0);
    
    printf("CCSEBAR: 0x%08x -> 0x%08x \r\n", CCSBAR_BASE_ADDR, CCSBAR_BASE_ADDR + 0x100000);
    printf("Kernel code + data: 0x%08x -> 0x%08x\n\r", 
           &kernel_offset, (unsigned int)&kernel_offset + 0x4000);
    printf("Kernel stack: 0x%08x -> 0x%08x\n\r", 
           &kernel_stack_start, (unsigned int)&kernel_stack_start + 0x1000);
}

static void display_config(void)
{
    printf("Free mem start: 0x%08x\n\r", &free_mem);
}

void kernel_kickstart(void)
{
    bsp_init();
    printf("BSP Initialized\n\r");

    display_config();

    printf("Launching main\n\r");

    main();

    printf("MiniKernel: HALTING\n\r");
}
