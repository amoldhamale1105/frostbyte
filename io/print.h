#ifndef PRINT_H
#define PRINT_H

#define BASE_NUMERIC_ASCII 48
#define BASE_CAPS_ALPHA_ASCII 65

#include <stdint.h>
#include <stdarg.h>

int printk(const char* fmt, ...);
char* itoa(int);
char* uitoa(uint32_t);
char* xtoa(uint64_t);

#endif