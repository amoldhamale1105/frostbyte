#include "libc.h"
#include <stddef.h>
#include <process/process.h>
#include <io/print.h>

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

struct Node *remove(struct List *list, const struct Node *node)
{
    struct Node* curr_node = list->head;
    struct Node* prev = NULL;

    while (curr_node != NULL)
    {
        if (node == curr_node){
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

bool contains(const struct List *list, const struct Node *node)
{
    const struct Node* curr_node = list->head;

    while (curr_node != NULL)
    {
        if (node == curr_node)
            return true;
        curr_node = curr_node->next;
    }
    
    return false;
}

struct Node *remove_evt(struct List* list, struct Node** const head_prev, int event)
{
    struct Node* node = list->head;
    struct Node* prev = NULL;

    if (head_prev != NULL){
        if (*head_prev != NULL){
            node = (*head_prev)->next;
            prev = *head_prev;
        }
    }
    
    while (node != NULL)
    {
        if (((struct Process*)node)->event == event){
            if (prev == NULL){
                if (list->head == node)
                    list->head = node->next;
                else
                    return NULL;
            }
            else
                prev->next = node->next;

            if (node->next == NULL)
                list->tail = prev;
            
            break;
        }
        prev = node;
        node = node->next;
    }
    if (head_prev != NULL)
        *head_prev = prev;
    
    return node;
}

struct Node *find_evt(const struct Node *head, int event)
{
    const struct Node* node = head;

    while (node != NULL)
    {
        if (((struct Process*)node)->event == event)
            break;
        node = node->next;
    }
    
    return (struct Node*)node;
}

bool empty(const struct List *list)
{
    return list->head == NULL;
}

void print_list(const struct List* list, const char* name)
{
    if (list->head == NULL){
        printk("%s empty\n", name);
        return;
    }
    struct Node* node = list->head;
    printk("%s => ");
    while (node != NULL)
    {
        printk("%d->", ((struct Process*)node)->pid);
        node = node->next;
    }
    printk("end\n");
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
