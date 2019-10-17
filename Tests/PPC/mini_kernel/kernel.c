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

#include "output.h"       /* output_string */
#include "qemu-ppce500.h" /* CONFIG_SYS_CCSRBAR_PHYS, CONFIG_SYS_CCSRBAR */
#include "tlb.h"          /* tlb_set_entry */
#include "mmu.h"          /* MASX_XX */


/* Test entry point */
extern void test_entry(void);

static void bsp_init(void)
{
    uint32_t i;

    /* Map CCSRBAR */
    tlb_set_entry(1, 1, 1, 0, 0, BOOKE_PAGESZ_1M, CONFIG_SYS_CCSRBAR,
                  MAS2_I | MAS2_G, CONFIG_SYS_CCSRBAR_PHYS, MAS3_SW | MAS3_SR);
    output_str("CCSRBAR Init\n", SERIAL);

    /* Map kernel init code */
    tlb_set_entry(0, 0, 1, 0, 0, BOOKE_PAGESZ_4K, 0x1000, MAS2_G, 0x1000,
        MAS3_SX | MAS3_SW | MAS3_SR);

    /* Map stack */
    tlb_set_entry(0, 1, 1, 0, 0, BOOKE_PAGESZ_4K, 0x2000, MAS2_G, 0x2000,
        MAS3_SX | MAS3_SW | MAS3_SR);

    /* Map 1:1 pages at 0x3000 for test code for 16K */
    for(i = 0; i < 4; ++i)
    {
        tlb_set_entry(0, 2 + i, 1, 0, 0, BOOKE_PAGESZ_4K, 0x3000 + i * 0x1000,
            MAS2_G, 0x3000 + i * 0x1000,
            MAS3_SX | MAS3_SW | MAS3_SR);
    }
    output_str("Test code mapped\n", SERIAL);

    /* Map 1:1 pages at 0x7000 for non coherent test data for 8K */
    for(i = 0; i < 2; ++i)
    {
        tlb_set_entry(0, 6 + i, 1, 0, 0, BOOKE_PAGESZ_4K, 0x7000 + i * 0x1000,
            MAS2_G, 0x7000 + i * 0x1000,
            MAS3_SW | MAS3_SR);
    }
    output_str("Test 1:1 non coherent data mapped\n", SERIAL);

    /* Map 1:1 pages at 0x9000 for coherent test data for 8K */
    for(i = 0; i < 2; ++i)
    {
        tlb_set_entry(0, 8 + i, 1, 0, 0, BOOKE_PAGESZ_4K, 0x9000 + i * 0x1000,
            MAS2_M | MAS2_G, 0x9000 + i * 0x1000,
            MAS3_SW | MAS3_SR);
    }
    output_str("Test 1:1 coherent data mapped\n", SERIAL);

    /* Map 1:-1 pages at 0xB000 for test data for 16K */
    for(i = 0; i < 4; ++i)
    {
        tlb_set_entry(0, 6 + i, 1, 0, 0, BOOKE_PAGESZ_4K, 0xB000 + i * 0x1000,
            MAS2_G, 0xE000 - i * 0x1000,
            MAS3_SW | MAS3_SR);
    }
    output_str("Test 1:-1 data mapped\n", SERIAL);

    /* Map 1:1 page at 0xF000 for test data for 4K cache inhibited */
    tlb_set_entry(0, 10, 1, 0, 0, BOOKE_PAGESZ_4K, 0xF000,
        MAS2_I | MAS2_G, 0xF000,
        MAS3_SW | MAS3_SR);
    output_str("Test 1:1 cache inhibited data mapped\n", SERIAL);

    /* Map 0x10000 page at 0x11000 test code for 4K cache inhibited coherent */
    tlb_set_entry(0, 11, 1, 0, 0, BOOKE_PAGESZ_4K, 0x10000,
        MAS2_I | MAS2_G | MAS2_M, 0x11000,
        MAS3_SW | MAS3_SR | MAS3_SX);
    output_str("Test 1:1 cache inhibited, coherent code mapped\n", SERIAL);

    /* Map 0x10000 page at 0x11000 test code for 4K cache inhibited coherent */
    tlb_set_entry(0, 11, 1, 0, 0, BOOKE_PAGESZ_4K, 0x11000,
        MAS2_W | MAS2_G | MAS2_M, 0x10000,
        MAS3_SW | MAS3_SR);
    output_str("Test 1:1 WT enabled, coherent data mapped\n", SERIAL);

}

void kernel_kickstart(void)
{
    bsp_init();
    output_str("BSP Initialize, jumping to test code\n", SERIAL);

    test_entry();

    while(1);
}
