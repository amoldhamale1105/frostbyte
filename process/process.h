#ifndef PROCESS_H
#define PROCESS_H

#include <irq/handler.h>
#include <fs/file.h>
#include <lib/libc.h>

struct Process
{
    /* Member needed for the scheduler to maintain a linked list of processes ready to run */
    struct Node* next; 
    int pid;
    int state;
    int event; /* Reason for wait used to wake up process on occurrence of specific events */
    uint64_t sp;
    uint64_t page_map;
    uint64_t stack;
    struct FileEntry* fd_table[100]; /* A user file desc table which contains pointers to global file table entries */
    struct ContextFrame* reg_context;
};

struct ProcessControl
{
    struct Process* curr_process;
    struct List ready_que;
    struct List wait_list;
    struct List zombies; /* Processes that have exited and awaiting resource cleanup */
};

#define STACK_SIZE PAGE_SIZE
#define TOTAL_PROCS 10
#define USERSPACE_CONTEXT_SIZE (12*8) /* 12 GPRs saved on the stack when context switch done by scheduler (see swap function) */
#define REGISTER_POSITION(addr, n) ((uint64_t)(addr) + (n*8)) /* Position of nth 8-byte register from current address */
#define MAX_OPEN_FILES 100

enum En_SleepEvent
{
    SLEEP_SYSCALL = 1,
    ZOMBIE_CLEANUP
};

enum En_ProcessState
{
    UNUSED = 0,
    INIT,
    RUNNING,
    READY,
    SLEEP,
    KILLED
};

void init_process(void);
void trigger_scheduler(void);
void swap(uint64_t* prev_sp_addr, uint64_t curr_sp);
void trap_return(void);
struct Process* get_curr_process();
void sleep(int event);
void wake_up(int event);
void exit(void);
void wait(int pid);
int fork(void);
int exec(struct Process* process, char* name);

#endif