/**
    Frostbyte kernel and operating system
    Copyright (C) 2023  Amol Dhamale <amoldhamale1105@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <io/uart.h>
#include <io/print.h>
#include <debug/debug.h>
#include <lib/lib.h>
#include <irq/handler.h>
#include <memory/memory.h>
#include <fs/file.h>
#include <process/process.h>
#include <irq/syscall.h>

/* A dummy non-zero global variable added for the kernel image to contain a data section
   In absence of data section, the image disregards the alignment padding after the rodata section for the disk image
   This results in ambiguity in the position of the FAT16 image start on the disk
   If the data section exists, the alignment padding (ALIGN(16) in linker script) will be written to disk image
   because the the next section (data) must start after the aligned end of rodata section only.
   Thus, the position of FAT16 image start and kernel end on disk (disk_img_end) can be correctly determined  */
int dummy_glob = 30;

void kmain(void)
{
    init_uart();
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
    printk("Shutdown complete\n");
    /* This will be conditional when support is added for hardware */
    printk("Press Ctrl-A X to exit QEMU monitor\n");
}