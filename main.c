#include "uart.h"
#include "print.h"
#include "debug.h"

void kmain(void)
{
    init_uart();
    printk("Welcome to Pious (An OS built for the Raspberry Pi 3b)\n");
    ASSERT(0);
    while(1);
}