#ifndef LIBC_H
#define LIBC_H

#include "stdint.h"

unsigned char get_el(void);
void delay(uint64_t value);
void out_word(uint64_t addr, uint32_t value);
uint32_t in_word(uint64_t addr);

void memset(void* dst, int value, unsigned int size);
void memcpy(void* dst, void* src, unsigned int size);
void memmove(void* dst, void* src, unsigned int size);
void memcmp(void* src1, void* src2, unsigned int size);

#endif