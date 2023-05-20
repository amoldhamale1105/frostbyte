#include "debug.h"
#include <io/print.h>

void error_check(char *filename, uint64_t line)
{
    printk("\r\n------------------------------\r\n");
    printk("          ERROR CHECK\r\n");
    printk("------------------------------\r\n");
    printk("Assertion Failed [%s: %u]\r\n", filename, line);

    while(1);
}