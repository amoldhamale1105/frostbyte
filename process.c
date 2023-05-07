#include "process.h"
#include "memory.h"
#include "debug.h"
#include "stddef.h"

static struct Process process_table[TOTAL_PROCS];
static int pid_num = 1;
void pstart(struct ContextFrame* ctx);

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
    /* Since this is the second process after the idle process, we verify that the second slot is allocated */
    ASSERT(process == process_table+1);

    /* Allocate memory for the kernel stack. Each process will have its own kernel stack */
    process->stack = (uint64_t)kalloc();
    ASSERT(process->stack != 0);
    memset((void*)process->stack, 0, STACK_SIZE);

    process->state = INIT;
    /* Assign PID 1 and post increment for the next process to receive the next number */
    process->pid = pid_num++;
    /* Get the context frame which is located at the top of the kernel stack */
    process->reg_context = (struct ContextFrame*)(process->stack + STACK_SIZE - sizeof(struct ContextFrame));
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
}

static void init_user_process(void)
{
    struct Process* process;
    process = alloc_new_process();
    ASSERT(process != NULL);

    ASSERT(setup_uvm((uint64_t)process->page_map, "INIT.BIN"));
}

void launch_process(void)
{
    /* Switch the page tables to point to the user process memory */
    switch_vm(process_table[1].page_map);
    /* Point the stack pointer to the context frame so that data we load there will be loaded into the registers */
    pstart(process_table[1].reg_context);
}

void init_process(void)
{
    init_idle_process();
    init_user_process();
    launch_process();
}
