#include <io/uart.h>
#include <io/print.h>
#include <debug/debug.h>
#include <lib/libc.h>
#include <irq/handler.h>
#include <memory/memory.h>
#include <fs/file.h>
#include <process/process.h>
#include <irq/syscall.h>

/* A dummy non-zero global variable added for the kernel image to contain a data section
   In absence of data section, the image disregards the alignment padding after the rodata section for the disk image
   This results in ambiguity in the position of the FAT16 image start on the disk
   If the data section exits, the alignment padding (ALIGN(16) in linker script) will be written to disk image
   because the the next section (data) must start after the aligned end of rodata section only.
   Thus, the position of FAT16 image start and kernel end on disk (disk_img_end) can be correctly determined  */
int dummy_glob = 30;

void kmain(void)
{
    init_uart();
    printk("\nWelcome to FrostByte (A minimalistic aarch64 kernel)\n\n");

    init_mem();
    init_fs();
    init_system_call();
    init_timer();
    init_interrupt_controller();
    enable_irq();
    init_process();
}

void shutdown_banner(void)
{
    printk("It's now safe to turn off your computer. Deja vu? ;)\n");
}