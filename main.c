#include "uart.h"
#include "print.h"
#include "debug.h"
#include "libc.h"
#include "handler.h"
#include "memory.h"

void kmain(void)
{
    init_uart();
    printk("Welcome to Pious (An OS built for the Raspberry Pi 3b)\n");
    printk("Current exception level is EL%u\n", (uint64_t)get_el());

    init_mem();
    init_timer();
    init_interrupt_controller();
    enable_irq();

    while(1);
}