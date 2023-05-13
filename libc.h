#ifndef LIBC_H
#define LIBC_H

#include "stdint.h"
#include "stdbool.h"

struct ProcNode
{
    struct ProcNode* next;
};

struct ReadyQue
{
    struct ProcNode* head;
    struct ProcNode* tail;
};

unsigned char get_el(void);
void delay(uint64_t value);
void out_word(uint64_t addr, uint32_t value);
uint32_t in_word(uint64_t addr);

void memset(void* dst, int value, unsigned int size);
void memcpy(void* dst, void* src, unsigned int size);
void memmove(void* dst, void* src, unsigned int size);
int memcmp(void* src1, void* src2, unsigned int size);

void enqueue(struct ReadyQue* que, struct ProcNode* pnode);
struct ProcNode* dequeue(struct ReadyQue* que);
bool empty(struct ReadyQue* que);

#endif