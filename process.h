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
};

#define TOTAL_PROCS 10

enum En_ProcessState
{
    UNUSED = 0,
    INIT,
    RUNNING
};

void init_process(void);

#endif