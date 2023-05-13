#ifndef PROCESS_H
#define PROCESS_H

#include "handler.h"
#include "file.h"
#include "libc.h"

struct Process
{
    /* Member needed for the scheduler to maintain a linked list of processes ready to run */
    struct ProcNode* next; 
    int pid;
    int state;
    uint64_t sp;
    uint64_t page_map;
    uint64_t stack;
    struct ContextFrame* reg_context;
};

struct ProcessControl
{
    struct Process* curr_process;
    struct ReadyQue ready_que;
};

#define STACK_SIZE PAGE_SIZE
#define TOTAL_PROCS 10
#define USERSPACE_CONTEXT_SIZE (12*8) /* 12 GPRs saved on the stack when context switch done by scheduler (see swap function) */
#define REGISTER_POSITION(addr, n) ((addr) + (n*8)) /* Position of nth 8-byte register from current address */

enum En_ProcessState
{
    UNUSED = 0,
    INIT,
    RUNNING,
    READY
};

void init_process(void);
void trigger_scheduler(void);
void swap(uint64_t* prev_sp_addr, uint64_t curr_sp);

#endif