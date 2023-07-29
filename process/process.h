#ifndef PROCESS_H
#define PROCESS_H

#include <irq/handler.h>
#include <fs/file.h>
#include <lib/lib.h>
#include "signal.h"

struct Process
{
    struct Node* next; /* Member needed for the scheduler to maintain a linked list of processes */
    char name[MAX_FILENAME_BYTES+1];
    int pid;
    int ppid;
    int state;
    bool daemon; /* Whether the process runs in the background as daemon */
    int event; /* Event a process is waiting on */
    uint64_t sp; /* Process kernel stack pointer */
    uint64_t page_map;
    uint64_t stack; /* Process kernel stack address */
    uint32_t signals; /* Pending signals bit map */
    struct FileEntry* fd_table[100]; /* A user file desc table which contains pointers to global file table entries */
    struct ContextFrame* reg_context;
    SIGHANDLER handlers[TOTAL_SIGNALS];
};

struct ProcessControl
{
    struct Process* curr_process;
    struct Process* fg_process; /* Current foreground process. This is not the same as current process */
    struct List ready_que;
    struct List wait_list;
    struct List zombies; /* Processes that have exited and awaiting resource cleanup */
};

#define STACK_SIZE PAGE_SIZE
#define PROC_TABLE_SIZE 100
#define USERSPACE_CONTEXT_SIZE (12*8) /* 12 GPRs saved on the stack when context switch done by scheduler (see swap function) */
#define REGISTER_POSITION(addr, n) ((uint64_t)(addr) + (n*8)) /* Position of nth 8-byte register from current address */
#define MAX_OPEN_FILES 100

enum En_SleepEvent
{
    NONE = -255,
    SLEEP_SYSCALL,
    ZOMBIE_CLEANUP,
    KEYBOARD_INPUT,
    DAEMON_INPUT
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
struct Process* get_curr_process(void);
struct Process *get_fg_process(void);
struct Process* get_process(int pid);
int get_proc_data(int pid, int* ppid, int* state, char* name, char* args_buf);
int get_active_pids(int* pid_list);
void switch_parent(int curr_ppid, int new_ppid);
void sleep(int event);
void wake_up(int event);
void exit(struct Process* process, bool sig_handler_req);
int wait(int pid);
int fork(void);
int exec(struct Process* process, char* name, const char* args[]);
int kill(struct Process *process, int signal);

#endif