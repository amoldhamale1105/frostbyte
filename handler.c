#include "stdint.h"
#include "print.h"

void handler(uint64_t id, uint64_t esr, uint64_t elr)
{
    switch (id)
    {
    case 1:
        printk("Sync exception at %x: %x\r\n", elr, esr);
        while(1);
        break;
    
    default:
        printk("Unknown exception\r\n");
        while(1);
        break;
    }
}