ENTRY(_entry)
SECTIONS
{
    kernel_offset = 0x0;    

    /* MiniKernel code start */
    . = kernel_offset;

    .boot . :
    {
        build/src/boot.S.o(.text)
    }

    .text . :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
        *(.data*)
        *(.sdata)
        *(.sdata*)
    }

    .bss :
    {
        _kernel_bss_start = .;
        *(.bss)
        *(.bss*)
        *(.sbss)
        *(.sbss*)
        *(COMMON)
        _kernel_bss_end = .;
    }

    /* Align stack bottom on long word boudaries */
    . = ALIGN(64K);

    kernel_stack_start = .;
    kernel_stack_base = . + 0x4000; /* Stack base */
    . = kernel_stack_base;

    . = ALIGN(4K);
    free_mem = .;

    
}