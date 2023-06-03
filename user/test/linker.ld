ENTRY(_start)

SECTIONS
{
    . = 0x400000;
    .text : 
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    . = ALIGN(16);
    .data :
    {
        *(.data)
    }

    .bss :
    {
        bss_start = .;
        *(.bss)
        bss_end = .;
    }
}