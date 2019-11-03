ENTRY(loader)                /* the name of the entry label */
SECTIONS
{
    /* Kernel starts at 0x80010000
     * Add the offset DO NOT FORGET TO CHANGE IT IN CONFIG.h AND LOADER.S
     */
    . = 0x80010000;

    /* Startup code stub */
    .stub ALIGN(4K) : 
    { 
        loader.o(.text) 
    } 

    /* Executable code */
    .text ALIGN(4K) :
    {
        *(.text)
    }

    /* Read-only data. */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    /* Read-write data */
    .data ALIGN(4K) :
    {
        *(.data)
    }
    _edata = .;
    /* FROM HERE EVERYTHING IS ZEROED */

    /* Read-write data and stack */
    .bss ALIGN(4K) : 
    { 
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }

    /* END OF ZEROING ZONE */
    _end = .;

    /* Pagen code */
    . = 0x80100000;
    _pagencodeaddr = .;

    _pagencodeaddr_phys = 0x80400000;

    /* Pagdis code */
    . = 0x80110000;
    _pagdiscodeaddr = .;    
 
    /* Put the stacks after the BSS. */
    . = 0x80120000;
    stacks_start = .;
    . = 0x80124000;
    usr_stack_end = .;
    . = 0x80128000;
    fiq_stack_end = .;
    . = 0x8012C000;
    irq_stack_end = .;
    . = 0x80130000;
    svc_stack_end = .;
    . = 0x80134000;
    abt_stack_end = .;
    . = 0x80138000;
    undef_stack_end = .;

    /* Memory regions */
    . = 0x80200000;
    reg_11_nc_ce_wt = .;
    . = 0x80240000;
    reg_11_c_ce_wt = .;
    . = 0x80280000;
    reg_1_inv_c_ce_wt = .;
    . = 0x802C0000;
    reg_11_c_ci_wt = .;
    . = 0x80300000;
    reg_1_1000_c_ce_wb = .;
    . = 0x80340000;

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/

    page_table_lvl1 = 0x80A00000;
    page_table_lvl2 = 0x80A04000;

    perif_gic_base_addr = 0x2C000000;
    perif_serial0_base_addr = 0x1C090000;
    perif_serial1_base_addr = 0x1C0A0000;
    perif_serial2_base_addr = 0x1C0B0000;
    perif_serial3_base_addr = 0x1C0C0000;


    
}