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

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct Page
{
    struct Page* next;
};

#define KERNEL_BASE     0xffff000000000000  /* Kernel base virtual address */
#define USERSPACE_BASE  0x0000000000400000  /* Userspace base virtual address */
#define USERSPACE_EXT   0x0000000000600000  /* Userspace extended virtual address base */

#define TO_VIRT(physical_addr)  ((uint64_t)physical_addr + KERNEL_BASE)
#define TO_PHY(virt_addr)       ((uint64_t)virt_addr - KERNEL_BASE)
#define TO_USER_VIRT(user_virt_base, kern_virt_base, kern_virt_addr) ({ \
                uint64_t user_virt_addr = 0; \
                if ((uint64_t)kern_virt_addr != 0) \
                    user_virt_addr = ((uint64_t)user_virt_base + ((uint64_t)(kern_virt_addr) - (uint64_t)(kern_virt_base))); \
                user_virt_addr; \
})

#define MEMORY_END          TO_VIRT(0X30000000)
#define PAGE_SIZE           0x200000 // 2M (2*1024*1024)
#define PAGE_TABLE_ENTRIES  512
#define PAGE_TABLE_SIZE     4096

#define ALIGN_UP(addr)      ((((uint64_t)addr + PAGE_SIZE - 1) >> 21) << 21)
#define ALIGN_DOWN(addr)    (((uint64_t)addr >> 21) << 21)

/* Translation table base register and directory tables GDT, UDT are 4k byte aligned hence bitwise AND with remaining bits will give the address of the next level table */
#define PAGE_DIR_ENTRY_ADDR(value)      ((uint64_t)value & 0xfffffffffffff000)
/* The middle directory table is 2M aligned (because of page size) hence the following bitmask to get the page address */
#define PAGE_TABLE_ENTRY_ADDR(value)    ((uint64_t)value & 0xffffffffffe00000)

#define ENTRY_VALID     (1 << 0)
#define TABLE_ENTRY     (1 << 1)
#define PAGE_ENTRY      (0 << 1)
#define ENTRY_ACCESSED  (1 << 10)
#define NORMAL_MEMORY   (1 << 2)
#define DEVICE_MEMORY   (0 << 2)
#define USER_MODE       (1 << 6)

struct Process;

void* kalloc(void);
void kfree(uint64_t addr);
void init_mem(void);
void free_uvm(uint64_t map);
bool setup_uvm(struct Process* process, char* program_filename);
bool copy_uvm(struct Process* process, uint64_t src_map, char* src_program_filename);
void switch_vm(uint64_t map);
uint64_t read_gdt(void);

#endif