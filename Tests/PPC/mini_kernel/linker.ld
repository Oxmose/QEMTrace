ENTRY(_entry)
SECTIONS
{
    /* MiniKernel code start */
    . = 0x0;

    .boot . :
    {
        boot.o(.text)
    }

    . = 0x00003000;

    .tests . :
    {
        tests.o(.text)
    }

    . = 0x1000;

    .text . :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss COMMON)
    }

    /* Align stack bottom on long word boudaries */
    . = 0x2000;

    stack_base = . + 0x1000; /* Stack base */

    . = stack_base;
}
