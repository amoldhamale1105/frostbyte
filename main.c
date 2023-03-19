#include "uart.h"
#include "print.h"
#include "debug.h"
#include "libc.h"

void kmain(void)
{
    init_uart();
    printk("Welcome to Pious (An OS built for the Raspberry Pi 3b)\n");
    printk("Current exception level is EL%u\n", (uint64_t)get_el());

    /* Test exception handler by accessing an out of range memory address */
    char* p = (char*)0xffff000000000000;
    *p = 1;

    printk("This message should not printed if an exception is raised for above memory access\n");

    while(1);
}