/**
    Frostbyte kernel and operating system
    Copyright (C) 2023  Amol Dhamale <amoldhamale1105@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LIB_H
#define LIB_H

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
struct Node* back(const struct List* list);
bool contains(const struct List* list, const struct Node* node);
struct Node *find(const struct Node *head, const struct Node *node);
bool empty(const struct List* list);
#ifdef DEBUG
void print_list(const struct List* list, const char* name);
#endif

/* Special functions for managing the process queues based on event occurence */

struct Node* remove_evt(struct List* list, struct Node** head_prev, int event);
struct Node* find_evt(const struct Node* head, int event);

int strlen(const char* str);

#endif