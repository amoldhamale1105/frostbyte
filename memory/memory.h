#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct Page
{
    struct Page* next;
};

#define KERNEL_BASE     0xffff000000000000
#define USERSPACE_BASE  0x400000

#define TO_VIRT(physical_addr)  ((uint64_t)physical_addr + KERNEL_BASE)
#define TO_PHY(virt_addr)       ((uint64_t)virt_addr - KERNEL_BASE)

#define MEMORY_END          TO_VIRT(0X30000000)
#define PAGE_SIZE           (2*1024*1024)
#define PAGE_TABLE_ENTRIES  512

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

void* kalloc(void);
void kfree(uint64_t addr);
void init_mem(void);
void free_vm(uint64_t map);
bool setup_uvm(uint64_t map, char* program_filename);
void switch_vm(uint64_t map);
uint64_t read_gdt(void);

#endif