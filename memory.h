#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"
#include "stddef.h"

struct Page
{
    struct Page* next;
};

#define KERNEL_BASE 0xffff000000000000

#define TO_VIRT(physical_addr)  ((uint64_t)physical_addr + KERNEL_BASE)
#define TO_PHY(virt_addr)       ((uint64_t)virt_addr - KERNEL_BASE)

#define MEMORY_END  TO_VIRT(0X30000000)
#define PAGE_SIZE   (2*1024*1024)

#define ALIGN_UP(addr)      ((((uint64_t)addr + PAGE_SIZE - 1) >> 21) << 21)
#define ALIGN_DOWN(addr)    (((uint64_t)addr >> 21) << 21)

void* kalloc(void);
void kfree(uint64_t addr);
void init_mem(void);

#endif