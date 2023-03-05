#include "uart.h"

void kmain(void)
{
    init_uart();
    write_string("Howdy coder?\n");
    while(1);
}