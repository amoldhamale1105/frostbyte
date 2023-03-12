#include "uart.h"
#include "print.h"

void kmain(void)
{
    init_uart();
    int value = 12434988;
    int another_value = -794;
    printk("Hello, coder!!\n");
    printk("Some number in decimal: %d in hex: %x\n", value, value);
    printk("Another number in decimal: %d in unsigned decimal: %u\n", another_value, another_value);
    while(1);
}