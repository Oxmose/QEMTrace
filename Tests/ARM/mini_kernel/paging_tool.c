/***************************************************************************//**
 * @file paging_tool.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/11/2019
 *
 * @version 1.0
 *
 * @brief Paging settings for Minikernel
 *
 * @details Contain MMU related functions such as page table creation.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include "common.h" /* Generic types */
#include "serial.h" /* Debug output */


/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

#define ARM_MMU_FLAG_PAGE_LARGE 0x1
#define ARM_MMU_FLAG_PAGE_SMALL 0x2

#define ARM_MMU_FLAG_STRONG_ORDER   0x0
#define ARM_MMU_FLAG_SHARED_DEV     (1 << 2)
#define ARM_MMU_FLAG_WT_NWA         (1 << 3)
#define ARM_MMU_FLAG_WB_NWA         ((1 << 3) | (1 << 2))
#define ARM_MMU_FLAG_CI             (1 << 6)
#define ARM_MMU_FLAG_WB_WA          ((1 << 6) | (1 << 3) | (1 << 2))
#define ARM_MMU_FLAG_NON_SHARED_DEV (1 << 7)
#define ARM_MMU_FLAG_RWX_ALL        ((1 << 5) | (1 << 4))

extern uint32_t page_table_lvl1;
extern uint32_t page_table_lvl2;

extern uint32_t _pagencodeaddr;
extern uint32_t _pagdiscodeaddr;    
 
extern uint32_t stacks_start;
extern uint32_t usr_stack_end;
extern uint32_t fiq_stack_end;
extern uint32_t irq_stack_end;
extern uint32_t svc_stack_end;
extern uint32_t abt_stack_end;
extern uint32_t undef_stack_end;

extern uint32_t reg_11_nc_ce_wt;
extern uint32_t reg_11_c_ce_wt;
extern uint32_t reg_1_inv_c_ce_wt;
extern uint32_t reg_11_c_ci_wt;
extern uint32_t reg_1_1000_c_ce_wb;

extern uint32_t perif_base_address;
extern uint32_t perif_gic_base_addr;
extern uint32_t perif_serial0_base_addr;
extern uint32_t perif_serial1_base_addr;
extern uint32_t perif_serial2_base_addr;
extern uint32_t perif_serial3_base_addr;

static uint32_t* last_l2_table = &page_table_lvl2;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define SAT(val, add, max) (max - val > add ? max : val + add)

/* 
 * WARNING ALL PAGES START AND END MUST BE ALIGNED ON 4K
 */
#if 0
#define GET_HEX_CHAR(ret, val) \
{                              \
    switch(val) {              \
        case 0:                \
            ret = '0';         \
            break;             \
        case 1:                \
            ret = '1';         \
            break;             \
        case 2:                \
            ret = '2';         \
            break;             \
        case 3:                \
            ret = '3';         \
            break;             \
        case 4:                \
            ret = '4';         \
            break;             \
        case 5:                \
            ret = '5';         \
            break;             \
        case 6:                \
            ret = '6';         \
            break;             \
        case 7:                \
            ret = '7';         \
            break;             \
        case 8:                \
            ret = '8';         \
            break;             \
        case 9:                \
            ret = '9';         \
            break;             \
        case 10:                \
            ret = 'A';         \
            break;             \
        case 11:                \
            ret = 'B';         \
            break;             \
        case 12:                \
            ret = 'C';         \
            break;             \
        case 13:                \
            ret = 'D';         \
            break;             \
        case 14:                \
            ret = 'E';         \
            break;             \
        case 15:                \
            ret = 'F';         \
            break;             \
        default:               \
            ret = 'u';         \
    }                          \
}

static void print_hex(uint32_t val)
{
    uint8_t i = 8;
    char newval[9] = {0};
    while(val)
    {
        GET_HEX_CHAR(newval[i--], val % 16);
        if(i == 4)
            newval[i--] = '_';
        val = val / 16;
    }
    serial_put_string(newval, 9);
}
static void print_table(void)
{
    uint32_t* l1_pgtable = &page_table_lvl1;
    uint32_t* entry;
    uint32_t i;
    uint32_t j;

    /* Set all empty MMU entries */
    for(i = 0; i < 4096; ++i)
    {
        if(l1_pgtable[i] != 0)
        {
            serial_put_string("== ", 3);
            serial_put_string("== 0x", 5);
            print_hex((uint32_t)&l1_pgtable[i]);
            serial_put_string("-> 0x", 3);
            print_hex(1024 * 1024 * i);
            serial_put_string("== 0x", 5);
            print_hex(l1_pgtable[i]);
            serial_put_string("\n", 1);
            entry = (uint32_t*)(l1_pgtable[i] & 0xFFFFFC00);
            for(j = 0; j < 256; ++j)
            {
                if(entry[j] != 0)
                {
                    serial_put_string("    0x", 6);
                    print_hex(entry[j]);
                    serial_put_string("\n", 1);
                }
            }
        }
    }
}
#endif
static void set_l2_pgt_entry(uint32_t* pgtable,
                             uint32_t start, 
                             uint32_t end, 
                             uint32_t start_virt,
                             uint32_t flags)
{
    uint32_t start_entry;

    /* Set the entries as much as we can */
    start_entry = (start & 0x0FFFFF) / 0x1000;

    for(start = start & 0xFFFFF000; 
        start < end && start_entry < 256; 
        start += 0x1000, start_virt += 0x1000)
    {
        pgtable[start_entry++] = (start_virt & 0xFFFFF000) | (flags & 0xFFF) | 0x2;
    }
}

static uint32_t create_l2_pgt_entry(uint32_t start, 
                                    uint32_t end, 
                                    uint32_t start_virt,
                                    uint32_t flags)
{
    uint32_t* l2_table;
    
    /* Create a new entry */
    l2_table = last_l2_table;

    set_l2_pgt_entry(l2_table, start, end, start_virt, flags);
    
    /* Update last_l2_table */
    last_l2_table += 256;

    return (uint32_t)l2_table;
}

static void set_l1_pgt_entry(uint32_t* table, 
                             uint32_t start, 
                             uint32_t end,
                             uint32_t start_virt,
                             uint32_t flags)
{
    uint32_t entry_id = start / 0x100000;
    while(start < end)
    {
        entry_id = start / 0x100000;
        
        if(table[entry_id] == 0)
        {
            /* Set a new entry */
            table[entry_id] = 0x9;
            table[entry_id] |= create_l2_pgt_entry(start, end, start_virt, 
                                                   flags);
        }
        else 
        {
             /* Modify an existing entry */
            set_l2_pgt_entry((uint32_t*)(table[entry_id] & 0xFFFFFC00), start, end, 
                             start_virt, flags);
        }

        start = SAT(start, 0x100000, 0xFFFFFFFF);
        start_virt = SAT(start_virt, 0x100000, 0xFFFFFFFF);
    }
}

void create_pgtable(void)
{
    uint32_t* l1_pgtable = &page_table_lvl1;
    uint32_t i;
    /* Set all empty MMU entries */
    for(i = 0; i < 4096; ++i)
    {
        l1_pgtable[i] = 0;
    }

    /* Map boot code 0x8000_0000 : 0x8010_0000 -> 1:1  WT */
    set_l1_pgt_entry(l1_pgtable, 0x80000000, 0x80100000, 0x80000000, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map pagen code 0x8010_0000 : 0x8040_0000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&_pagencodeaddr, (uint32_t)&_pagdiscodeaddr, 0x80400000,
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map pagedis code 0x8011_0000 : 0x8011_0000 -> 1:1  WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&_pagdiscodeaddr, 0x80120000, 0x80110000, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map pagedis code 0x8011_0000 : 0x8011_0000 -> 1:1  WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&_pagdiscodeaddr, 0x80120000, 0x80110000, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);

    /* Map stack 0x8012_0000 : 0x8012_4000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&stacks_start, (uint32_t)&usr_stack_end, (uint32_t)&stacks_start, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map stack 0x8012_4000 : 0x8012_8000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&usr_stack_end, (uint32_t)&fiq_stack_end, (uint32_t)&usr_stack_end, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map stack 0x8012_8000 : 0x8012_C000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&fiq_stack_end, (uint32_t)&irq_stack_end, (uint32_t)&fiq_stack_end, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map stack 0x8012_C000 : 0x8013_0000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&irq_stack_end, (uint32_t)&svc_stack_end, (uint32_t)&irq_stack_end, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map stack 0x8013_0000 : 0x8013_4000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&svc_stack_end, (uint32_t)&abt_stack_end, (uint32_t)&svc_stack_end,
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map stack 0x8013_4000 : 0x8013_8000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&abt_stack_end, (uint32_t)&undef_stack_end, (uint32_t)&abt_stack_end, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);

    /* Map test data 0x8020_0000 : 0x8024_0000 -> 1:1 NC (TODO) WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&reg_11_nc_ce_wt, (uint32_t)&reg_11_c_ce_wt, (uint32_t)&reg_11_nc_ce_wt, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    /* Map test data 0x8024_0000 : 0x8028_0000 -> 1:1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&reg_11_c_ce_wt, (uint32_t)&reg_1_inv_c_ce_wt, (uint32_t)&reg_11_c_ce_wt, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);

    /* Map test data 0x8028_0000 : 0x802C_0000 -> 1:-1 WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&reg_1_inv_c_ce_wt, 
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x10000, 
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x30000, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    set_l1_pgt_entry(l1_pgtable, 
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x10000,
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x20000, 
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x20000, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    set_l1_pgt_entry(l1_pgtable, 
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x20000,
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x30000, 
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x10000, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);
    set_l1_pgt_entry(l1_pgtable, 
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x30000,
                     (uint32_t)(&reg_1_inv_c_ce_wt) + 0x40000, 
                     (uint32_t)&reg_1_inv_c_ce_wt, 
                     ARM_MMU_FLAG_WT_NWA | ARM_MMU_FLAG_RWX_ALL);

    /* Map test data 0x802C_0000 : 0x8030_0000 -> 1:1 CI WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&reg_11_c_ci_wt, (uint32_t)&reg_1_1000_c_ce_wb, (uint32_t)&reg_11_c_ci_wt, 
                     ARM_MMU_FLAG_CI | ARM_MMU_FLAG_RWX_ALL);
    /* Map test data 0x8030_0000 : 0x8034_0000 -> 1:1+0x1000 WB */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&reg_1_1000_c_ce_wb, 0x80340000, (uint32_t)(&reg_1_1000_c_ce_wb) + 0x1000, 
                     ARM_MMU_FLAG_WB_WA | ARM_MMU_FLAG_RWX_ALL);

    /* Map peripherals 1:1 -> CI WT */
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&perif_gic_base_addr, 
                     (uint32_t)(&perif_gic_base_addr) + 0x00010000,
                     (uint32_t)&perif_gic_base_addr, 
                     ARM_MMU_FLAG_CI | ARM_MMU_FLAG_RWX_ALL);
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&perif_serial0_base_addr, 
                     (uint32_t)(&perif_serial0_base_addr) + 0x00010000,
                     (uint32_t)&perif_serial0_base_addr, 
                     ARM_MMU_FLAG_CI | ARM_MMU_FLAG_RWX_ALL);
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&perif_serial1_base_addr, 
                     (uint32_t)(&perif_serial1_base_addr) + 0x00010000,
                     (uint32_t)&perif_serial1_base_addr, 
                     ARM_MMU_FLAG_CI | ARM_MMU_FLAG_RWX_ALL);
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&perif_serial2_base_addr, 
                     (uint32_t)(&perif_serial2_base_addr) + 0x00010000,
                     (uint32_t)&perif_serial2_base_addr, 
                     ARM_MMU_FLAG_CI | ARM_MMU_FLAG_RWX_ALL);
    set_l1_pgt_entry(l1_pgtable, (uint32_t)&perif_serial3_base_addr, 
                     (uint32_t)(&perif_serial3_base_addr) + 0x00010000,
                     (uint32_t)&perif_serial3_base_addr, 
                     ARM_MMU_FLAG_CI | ARM_MMU_FLAG_RWX_ALL);
}