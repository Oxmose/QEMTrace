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
 
    /* Put the kernel stack after the BSS. */
    kernel_stack_start = .;
    . += 0x10000;
    kernel_stack_end = .;

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/
    periph_base_address = .;

    
}