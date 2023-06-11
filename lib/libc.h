#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>
#include <stdbool.h>

struct Node
{
    struct Node* next;
};

struct List
{
    struct Node* head;
    struct Node* tail;
};

unsigned char get_el(void);
void delay(uint64_t value);
void out_word(uint64_t addr, uint32_t value);
uint32_t in_word(uint64_t addr);

void memset(void* dst, int value, unsigned int size);
void memcpy(void* dst, void* src, unsigned int size);
void memmove(void* dst, void* src, unsigned int size);
int memcmp(void* src1, void* src2, unsigned int size);

void push_back(struct List* list, struct Node* node);
struct Node* pop_front(struct List* list);
/* Special function for the sleep and wakeup implementation */
struct Node* remove(struct List* list, int event);
bool empty(struct List* list);

int strlen(const char* str);

#endif