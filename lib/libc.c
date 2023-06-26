#include "libc.h"
#include <stddef.h>
#include <process/process.h>

void push_back(struct List *list, struct Node *node)
{
    if (list->head == NULL){
        list->head = list->tail = node;
        node->next = NULL;
        return;
    }

    list->tail->next = node;
    node->next = NULL;
    list->tail = node;
}

struct Node *pop_front(struct List *list)
{
    struct Node* node = NULL;

    if (list->head != NULL){
        node = list->head;
        list->head = node->next;
    }

    return node;
}

struct Node *erase(struct List *list, struct Node *node)
{
    struct Node* curr_node = list->head;
    struct Node* prev = NULL;

    while (curr_node != NULL)
    {
        if ((uint64_t)node == (uint64_t)curr_node){
            if (prev == NULL)
                list->head = curr_node->next;
            else
                prev->next = curr_node->next;

            if (curr_node->next == NULL)
                list->tail = prev;
            
            break;
        }
        prev = curr_node;
        curr_node = curr_node->next;
    }
    
    return curr_node;
}

struct Node* front(const struct List* list)
{
    return list->head;
}

struct Node *remove(struct List *list, int event)
{
    struct Node* node = list->head;
    struct Node* prev = NULL;

    while (node != NULL)
    {
        if (((struct Process*)node)->event == event){
            if (prev == NULL)
                list->head = node->next;
            else
                prev->next = node->next;

            if (node->next == NULL)
                list->tail = prev;
            
            break;
        }
        prev = node;
        node = node->next;
    }
    
    return node;
}

bool empty(struct List *list)
{
    return list->head == NULL;
}

int strlen(const char *str)
{
    int len = 0;
    while (*(str+len) != '\0')
    {
        len++;
    }
    return len;
}
