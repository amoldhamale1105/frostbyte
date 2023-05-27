#ifndef LIB_H
#define LIB_H

#define BASE_NUMERIC_ASCII 48
#define BASE_CAPS_ALPHA_ASCII 65

#include "stdint.h"
#include "stdarg.h"

int printf(const char* fmt, ...);
char* itoa(int);
char* uitoa(uint32_t);
char* xtoa(uint64_t);

int writeu(char* buf, int buf_size);
void sleepu(uint64_t ticks_10ms);
int open_file(char* filename);
int close_file(int fd);

#endif