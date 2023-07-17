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
struct Node* remove(struct List* list, const struct Node* node);
struct Node* front(const struct List* list);
bool contains(const struct List* list, const struct Node* node);
bool empty(const struct List* list);
/* Debugging utility function for kernel list data structures */
void print_list(const struct List* list, const char* name);

/* Special functions for managing the process queues based on event occurence */

struct Node* remove_evt(struct List* list, struct Node** head_prev, int event);
struct Node* find_evt(const struct Node* head, int event);

int strlen(const char* str);

#endif