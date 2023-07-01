#include "process.h"
#include <memory/memory.h>
#include <debug/debug.h>
#include <stddef.h>
#include <io/print.h>

static struct Process process_table[PROC_TABLE_SIZE];
static int pid_num = 1;
static struct ProcessControl pc;

static struct Process* find_unused_slot(void)
{
    struct Process* process = NULL;

    for (int i = 0; i < PROC_TABLE_SIZE; i++)
    {
        if (process_table[i].state == UNUSED){
            process = process_table + i;
            break;
        }
    }
    
    return process;
}

static struct Process* alloc_new_process(void)
{
    struct Process* process;

    process = find_unused_slot();
    if (process == NULL)
        return NULL;

    memset(process->name, 0, sizeof(process->name));
    /* Allocate memory for the kernel stack. Each process will have its own kernel stack */
    process->stack = (uint64_t)kalloc();
    ASSERT(process->stack != 0);
    memset((void*)process->stack, 0, STACK_SIZE);

    process->state = INIT;
    /* Assign PID 1 and post increment for the next process to receive the next number */
    process->pid = pid_num++;
    /* Get the context frame which is located at the top of the kernel stack */
    process->reg_context = (struct ContextFrame*)(process->stack + STACK_SIZE - sizeof(struct ContextFrame));
    /* Set the stack pointer to 12 GPRs below the context frame where the userspace context is saved */
    process->sp = (uint64_t)process->reg_context - USERSPACE_CONTEXT_SIZE;
    /* By moving 11 registers up the stack, we reach location of register x30 where we store address of trap_return 
       Since return addresses are stored in x30 in aarch64, control reaches there after executing the ret instruction in swap function
       The elr and spsr address set in the register context below will then enable trap_return to switch to EL0 correctly post an eret instruction
       NOTE This is only applicable to the first run. In subsequent runs, the process will resume execution from the point of interruption */
    *(uint64_t*)(REGISTER_POSITION(process->sp, 11)) = (uint64_t)trap_return;
    /* The return address should be set to the userspace base address */
    process->reg_context->elr = USERSPACE_BASE;
    /* In current version of the kernel, all regions (text, stack, data) of a process are expected to lie in the same 2M page  
       Hence, set the stack pointer to the top of the page from where it can grow downwards */
    process->reg_context->sp0 = USERSPACE_BASE + PAGE_SIZE;
    /* Set pstate mode field to 0 (EL0) and DAIF bits to 0 which means no masking of interrupts i.e. interrupts enabled */
    process->reg_context->spsr = 0;
    /* Allocate memory for page map which stores GDT (global directory table) for userspace */
    process->page_map = (uint64_t)kalloc();
    ASSERT(process->page_map != 0);
    memset((void*)process->page_map, 0, PAGE_SIZE);

    return process;
}

static void init_idle_process(void)
{
    struct Process* process;
    /* Find an unused slot in the process table */
    process = find_unused_slot();
    /* Since this is the first process, ensure that the first slot is received */
    ASSERT(process == process_table);

    process->state = RUNNING;
    process->pid = 0;
    /* Since this is the first process of the system, page map is initialized with current val of TTBR0 register */
    process->page_map = TO_VIRT(read_gdt());
    pc.curr_process = process;
}

static void init_user_process(void)
{
    struct Process* process;
    const char* filename = "INIT.BIN";
    
    process = alloc_new_process();
    ASSERT(process != NULL);

    ASSERT(setup_uvm(process, (char*)filename));
    memcpy(process->name, (char*)filename, strlen(filename)-(MAX_EXTNAME_BYTES+1));
    process->ppid = 0;
    process->state = READY;
    /* Initialize signal handlers for the init process */
    init_handlers(process);
    push_back(&pc.ready_que, (struct Node*)process);
}

void init_process(void)
{
    pc.ready_que.head = pc.ready_que.tail = NULL;
    init_idle_process();
    init_def_handlers(&pc);
    init_user_process();
}

static void switch_process(struct Process* prev, struct Process* curr)
{
    /* Switch the page tables to point to the new user process memory */
    switch_vm(curr->page_map);
    /* Swap the currently running process with the new process chosen by the scheduler */
    swap(&prev->sp, curr->sp);
    /* The previous process will resume execution here once swapped in unless it's the first time it's running
       In the first run, x30 will point to trap_return. In subsequent scheduling cycles, the return address will point here
       User processes will have this address in x30 and won't be overwritten with trap_return address in subsequent cycles
       The idle process (PID 0) will always resume here even the first time because there's no redirection defined to trap_return for it
       The rationale is that idle process always runs in kernel space i.e. EL1 not in userspace unlike other processes */
}

static void schedule(void)
{
    struct Process* old_process = pc.curr_process;
    struct Process* new_process;

    /* While returning to user mode from kernel mode, check for any pending signals on the process about to be scheduled */
    if (!empty(&pc.ready_que))
        check_pending_signals((struct Process*)front(&pc.ready_que));
    /* In absence of other processes in the queue, the idle process will be chosen to run
       Otherwise pick the process at the head of the ready queue */
    new_process = empty(&pc.ready_que) ? process_table : (struct Process*)pop_front(&pc.ready_que);

    new_process->state = RUNNING;
    pc.curr_process = new_process;

    switch_process(old_process, new_process);
}

void trigger_scheduler(void)
{
    struct Process* process;

    /* Return and continue running the same process if the ready queue is empty */
    if (empty(&pc.ready_que))
        return;
    /* The current process state needs to be changed from running to ready */
    process = pc.curr_process;
    process->state = READY;

    /* The idle process (PID 0) is run by default and is also not appended to the ready queue */
    if (process->pid != 0)
        push_back(&pc.ready_que, (struct Node*)process);

    schedule();
}

struct Process *get_curr_process(void)
{
    return pc.curr_process;
}

struct Process *get_process(int pid)
{
    struct Process* process = NULL;

    for (int i = 1; i < PROC_TABLE_SIZE; i++)
    {
        if (process_table[i].state != UNUSED && process_table[i].pid == pid){
            process = &process_table[i];
            break;
        }
    }
    return process;
}

void get_proc_data(int pid, int *ppid, int *state, char *name)
{
    for (int i = 1; i < PROC_TABLE_SIZE; i++)
    {
        if (process_table[i].state != UNUSED && process_table[i].pid == pid){
            if (ppid != NULL)
                *ppid = process_table[i].ppid;
            if (state != NULL)
                *state = process_table[i].state;
            if (name != NULL)
                memcpy(name, process_table[i].name, strlen(process_table[i].name));
            break;
        }
    }
}

int get_active_pids(int* pid_list)
{
    int count = 0;
    /* Omit the idle process which occupies the first slot in the process table
       The idle process should be always runnning in kernel context until the system is shutdown */
    for(int i = 1; i < PROC_TABLE_SIZE; i++)
    {
        if (process_table[i].state != UNUSED)
            pid_list[count++] = process_table[i].pid;
    }

    return count;
}

void switch_parent(int curr_ppid, int new_ppid)
{
    for(int i = 1; i < PROC_TABLE_SIZE; i++)
    {
        /* Reassign parent for all children which have current parent with curr_ppid */
        if (process_table[i].state != UNUSED && process_table[i].ppid == curr_ppid)
            process_table[i].ppid = new_ppid;
    }
}

void sleep(int event)
{
    struct Process* process;

    process = pc.curr_process;
    process->state = SLEEP;
    /* Save the reason of wait which can used in wake_up to selectively wake up processes based on occurred events */
    process->event = event;

    /* Enqueue the process on the wait list so that it cannnot be rescheduled until woken up and placed on ready queue */
    push_back(&pc.wait_list, (struct Node*)process);
    /* Call the scheduler to replace the current process (which just slept) with other process on the ready queue */
    schedule();
}

void wake_up(int event)
{
    struct Process* process;

    /* remove first process waiting on event from the wait list */
    process = (struct Process*)remove(&pc.wait_list, event);
    /* Place it on the ready queue and check if any other processes waiting on the same event
       If they're sleeping, remove from wait list and place them on the ready queue as well */
    while (process != NULL)
    {
        process->state = READY;
        push_back(&pc.ready_que, (struct Node*)process);
        process = (struct Process*)remove(&pc.wait_list, event);
    }
}

void exit(struct Process* process, bool sig_handler_req)
{
    if (process == NULL || process->state == UNUSED || process->state == KILLED)
        return;

    /* Set the state to killed and event to PID for the wait function to sweep it later */
    process->state = KILLED;
    process->event = process->pid;
    /* Inform the parent about death of child */
    struct Process* parent = get_process(process->ppid);
    if (parent != NULL)
        parent->signals |= (1 << SIGCHLD);

    push_back(&pc.zombies, (struct Node*)process);

    /* Wake up the process sleeping in wait to clean up this zombie process */
    wake_up(ZOMBIE_CLEANUP);

    /* Put off scheduling if invoked by a signal handler because it will have work to do */
    if (!sig_handler_req)
        schedule();
}

void wait(int pid)
{
    struct Process* zombie;

    while (1)
    {
        if (!empty(&pc.zombies)){
            zombie = (struct Process*)remove(&pc.zombies, pid);
            if (zombie != NULL){
                /* There is a chance a signal handler cleaned up the process resources already */
                if (zombie->state != KILLED)
                    break;
                kfree(zombie->stack);
                free_uvm(zombie->page_map);
                /* Decrement ref counts of all files left open by the zombie */
                for(int i = 0; i < MAX_OPEN_FILES; i++)
                {
                    if (zombie->fd_table[i] != NULL){
                        zombie->fd_table[i]->ref_count--;
                        zombie->fd_table[i]->inode->ref_count--;
                        /* Note that we can't release the inode based on only the file entry ref count because other file entries could be pointing to it
                           Hence the in core inode should be released (entry set to NULL) only if the inode ref count is zero */
                        if (zombie->fd_table[i]->inode->ref_count == 0)
                            zombie->fd_table[i]->inode = NULL;
                    }
                }
                /* No need to clear the memory used in the process table slot used by the zombie because that will anyway be overwritten
                   when allocating a new process. The find_unused_slot() function only cares if the state is unused or not to reallocate
                   a particular slot, so why not save some unnecessay CPU cycles requried for memsetting the slot area to 0 */
                zombie->state = UNUSED;
                break;
            }
        }

        sleep(ZOMBIE_CLEANUP);
    }
}

int fork(void)
{
    struct Process* process;

    /* Allocate a new child process */
    process = alloc_new_process();
    if (process == NULL)
        return -1;
    
    /* Copy the process name and set parent process ID */
    memcpy(process->name, pc.curr_process->name, sizeof(process->name));
    process->ppid = pc.curr_process->pid;
    /* Copy the text, data, stack and other regions of the parent to the child process' memory
       We copy only one page because the current design of the kernel holds all process regions in a single page */
    if (!copy_uvm(process->page_map, pc.curr_process->page_map, PAGE_SIZE))
        return -1;

    /* Replicate the parent file descriptor table for the child since it shares all open files with the parent 
       Increment the global file table entry ref count of open files. The inode ref count will be incremented as usual */
    memcpy(process->fd_table, pc.curr_process->fd_table, MAX_OPEN_FILES * sizeof(struct FileEntry*));
    for(int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (process->fd_table[i] != NULL){
            process->fd_table[i]->ref_count++;
            process->fd_table[i]->inode->ref_count++;
        }
    }

    /* Copy the context frame so that the child process also resumes at the point after the fork call */
    memcpy(process->reg_context, pc.curr_process->reg_context, sizeof(struct ContextFrame));
    /* Initialize signal handlers for the child process */
    init_handlers(process);
    /* Set the return value for child process to 0 */
    process->reg_context->x0 = 0;
    process->state = READY;
    push_back(&pc.ready_que, (struct Node*)process);

    /* The parent process which called fork will be returned the child process PID */
    return process->pid;
}

int exec(struct Process* process, char* name, const char* args[])
{
    int fd;
    uint32_t size;

    fd = open_file(process, name);
    if (fd == -1)
        return -1;

    /* Get the size and count of passed arguments for the new program */
    int arg_count = 0;
    int arg_size = 0;
    if (args != NULL){
        while (args[arg_count] != NULL)
        {
            arg_size += (strlen(args[arg_count])+1);
            arg_count++;
        }
    }
    arg_size += (strlen(name)+1);

    /* Use the bottom of the allocated stack space for saving programs args instead of current stack pointer address
       This is done to avoid overwriting kernel stack at stack pointer location which contains parent call stack */
    char* arg_val_ks = (char*)process->stack;
    
    /* Copy the program name and args to process kernel stack */
    int arg_len[arg_count+1];
    arg_len[0] = strlen(name);
    memcpy(arg_val_ks, name, arg_len[0]);
    arg_val_ks[arg_len[0]] = 0;
    arg_val_ks += (arg_len[0]+1);
    for(int i = 0; i < arg_count; i++)
    {
        arg_len[i+1] = strlen(args[i]);
        memcpy(arg_val_ks, (char*)args[i], arg_len[i+1]);
        arg_val_ks[arg_len[i+1]] = 0;
        arg_val_ks += (arg_len[i+1]+1);
    }
    /* Set new name in the process table entry. NOTE Parent process ID would remain the same */
    memset(process->name, 0, sizeof(process->name));
    memcpy(process->name, name, strlen(name)-(MAX_EXTNAME_BYTES+1));
    /* In exec call, the regions of the current process are overwritten with the regions of the new process and PID remains the same.
       Hence there's no need to allocate new memory for the new program */
    memset((void*)USERSPACE_BASE, 0, PAGE_SIZE);
    size = get_file_size(process, fd);
    /* We use the userspace virt address as buffer because memory was previously allocated for the process which called exec */
    size = read_file(process, fd, (void*)USERSPACE_BASE, size);
    /* Here if the exec operation fails, only option is to exit because we've cleared the regions of original process */
    if (size == UINT32_MAX)
        exit(process, false);

    close_file(process, fd);
    /* Clear any previously set custom handlers and initialize default signal handlers for the new process */
    memset(process->handlers, 0, sizeof(SIGHANDLER)*TOTAL_SIGNALS);
    init_handlers(process);
    /* Clear the previous process' context frame since we don't return to it */
    memset(process->reg_context, 0, sizeof(struct ContextFrame));
    /* The return address should be set to start of text section of new process i.e. the userspace base address */
    process->reg_context->elr = USERSPACE_BASE;
    /* Set the user program stack pointer to highest page address from where it can grow downwards */
    process->reg_context->sp0 = USERSPACE_BASE + PAGE_SIZE;
    /* Set pstate mode field to 0 (EL0) and DAIF bits to 0 which means no masking of interrupts i.e. interrupts enabled */
    process->reg_context->spsr = 0;
    /* Save arg count in x2 since x0 will be overwritten by the syscall return value when this function returns
       Before calling main, userspace programs can move this value to x0 to be identified as first arg to main */
    process->reg_context->x2 = arg_count+1;
    /* Make room for program arg pointers and content on the userspace stack */
    process->reg_context->sp0 -= (arg_count+1)*8;
    int64_t* arg_ptr = (int64_t*)process->reg_context->sp0;
    process->reg_context->sp0 -= UPPER_BOUND(arg_size, 8);

    /* Copy program arguments from the kernel stack to the user stack for the process to access */
    char* arg_val = (char*)process->reg_context->sp0;
    arg_val_ks = (char*)process->stack;

    memcpy(arg_val, arg_val_ks, arg_len[0]);
    *arg_ptr = (int64_t)arg_val;
    arg_ptr++;
    arg_val += (arg_len[0]+1);
    arg_val_ks += (arg_len[0]+1);
    for(int i = 1; i <= arg_count; i++)
    {
        memcpy(arg_val, arg_val_ks, arg_len[i]);
        *arg_ptr = (int64_t)arg_val;
        arg_ptr++;
        arg_val += (arg_len[i]+1);
        arg_val_ks += (arg_len[i]+1);
    }

    /* Save the argument addresses location on the stack to x1 to be used as second argument to main */
    process->reg_context->x1 = (int64_t)arg_ptr - (arg_count+1)*8;

    return 0;
}

int kill(struct Process *process, int signal)
{
    if (process == NULL || process->state == UNUSED)
        return -1;
    if (signal <= 0 || signal > TOTAL_SIGNALS-1)
        return -1;
    process->signals |= (1 << signal);
    return 0;
}
