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
