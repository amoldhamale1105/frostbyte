#ifndef PROCESS_H
#define PROCESS_H

#include "handler.h"
#include "file.h"
#include "libc.h"

struct Process
{
    int pid;
    int state;
    uint64_t page_map;
    uint64_t stack;
    struct ContextFrame* reg_context;
};

#define STACK_SIZE PAGE_SIZE
#define TOTAL_PROCS 10

enum En_ProcessState
{
    UNUSED = 0,
    INIT,
    RUNNING,
    READY
};

void init_process(void);

#endif