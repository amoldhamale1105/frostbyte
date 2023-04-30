#include "uart.h"
#include "print.h"
#include "debug.h"
#include "libc.h"
#include "handler.h"
#include "memory.h"
#include "file.h"
#include "process.h"

/* A dummy non-zero global variable added for the kernel image to contain a data section
   TODO This can be removed once a global variable is added anywhere else in the kernel source
   In absence of data section, the image disregards the alignment padding after the rodata section for the disk image
   This results in ambiguity in the position of the FAT16 image start on the disk
   If the data section exits, the alignment padding (ALIGN(16) in linker script) will be written to disk image
   because the the next section (data) must start after the aligned end of rodata section only.
   Thus, the position of FAT16 image start and kernel end on disk (disk_img_end) can be correctly determined  */
int dummy_glob = 30;

void kmain(void)
{
    init_uart();
    printk("Welcome to Pious (An OS built for the Raspberry Pi 3b)\n");
    printk("Current exception level is EL%u\n", (uint64_t)get_el());

    init_mem();
    init_fs();
    init_timer();
    init_interrupt_controller();
    init_process();
    enable_irq();
}