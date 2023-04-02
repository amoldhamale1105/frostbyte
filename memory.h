#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"

#define KERNEL_BASE 0xffff000000000000

#define TO_VIRT(physical_addr)  ((uint64_t)physical_addr + KERNEL_BASE)
#define TO_PHY(virt_addr)       ((uint64_t)virt_addr - KERNEL_BASE)

#endif