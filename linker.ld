ENTRY(_start)

SECTIONS
{
    . = 0xffff000000080000;
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
    disk_img_end = .;

    .bss :
    {
        bss_start = .;
        *(.bss)
        bss_end = .;
    }

    kern_end = .;
}