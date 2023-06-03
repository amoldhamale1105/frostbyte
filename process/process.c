#include "process.h"
#include <memory/memory.h>
#include <debug/debug.h>
#include <stddef.h>

static struct Process process_table[TOTAL_PROCS];
static int pid_num = 1;
static struct ProcessControl pc;

static struct Process* find_unused_slot(void)
{
    struct Process* process = NULL;

    for (int i = 0; i < TOTAL_PROCS; i++)
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
    /* In pious, the all regions (text, stack, data) of a process are expected to lie in the same 2M page  
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
    process = alloc_new_process();
    ASSERT(process != NULL);

    ASSERT(setup_uvm(process, "INIT.BIN"));

    process->state = READY;
    push_back(&pc.ready_que, (struct Node*)process);
}

void init_process(void)
{
    pc.ready_que.head = pc.ready_que.tail = NULL;
    init_idle_process();
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

struct Process *get_curr_process()
{
    return pc.curr_process;
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

void exit(void)
{
    struct Process* process = pc.curr_process;
    
    /* Set the state to killed and event to PID for the wait function to sweep it later */
    process->state = KILLED;
    process->event = process->pid;

    push_back(&pc.zombies, (struct Node*)process);

    /* Wake up the process sleeping in wait to clean up this zombie process */
    wake_up(ZOMBIE_CLEANUP);

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
                ASSERT(zombie->state == KILLED);
                kfree(zombie->stack);
                free_vm(zombie->page_map);
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

    /* Set the return value for child process to 0 */
    process->reg_context->x0 = 0;
    process->state = READY;
    push_back(&pc.ready_que, (struct Node*)process);

    /* The parent process which called fork will be returned the child process PID */
    return process->pid;
}
