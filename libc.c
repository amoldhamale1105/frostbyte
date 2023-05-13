#include "libc.h"
#include "stddef.h"

void enqueue(struct ReadyQue *que, struct ProcNode *pnode)
{
    if (que->head == NULL){
        que->head = que->tail = pnode;
        pnode->next = NULL;
        return;
    }

    que->tail->next = pnode;
    pnode->next = NULL;
    que->tail = pnode;
}

struct ProcNode *dequeue(struct ReadyQue *que)
{
    struct ProcNode* node = NULL;

    if (que->head != NULL){
        node = que->head;
        que->head = node->next;
    }

    return node;
}

bool empty(struct ReadyQue *que)
{
    return que->head == NULL;
}
