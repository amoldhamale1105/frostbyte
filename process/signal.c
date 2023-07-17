#include <memory/memory.h>
#include "signal.h"
#include "process.h"

/* Maintain a table of default handlers to replace custom handler after single invokation */
static SIGHANDLER def_handlers[TOTAL_SIGNALS];
/* Target process pointer with global scope within this file for default handlers */
static struct Process* target_proc = NULL;
/* Process control data structure with global scope within this file for default handlers */
static struct ProcessControl* pc = NULL;
/* Global proxy signal handler managing custom handler invokation followed by context restoration */
static SIGHANDLER_PROXY proxy_handler = NULL;

void check_pending_signals(struct Process* process)
{
    if (process != NULL && process->signals > 0){
        for(int i = 0; i < TOTAL_SIGNALS; i++)
        {
            /* Whether the process is still alive to check for precence of other signals */
            if (process->state == UNUSED || process->state == KILLED)
                break;
            if (process->signals & (1 << i)){
                if (process->handlers[i] != NULL){
                    /* Custom handlers should be invoked in user mode only which can be deduced from the handler address */
                    if (!((uint64_t)(process->handlers[i]) & KERNEL_BASE)){
                        switch_vm(process->page_map);
                        int64_t el0_addr = process->reg_context->elr;
                        /* Enable the proxy handler to run on eret which will invoke custom handler and restore previous context */
                        process->reg_context->elr = (int64_t)proxy_handler;
                        /* Save data required for proxy handler on user stack since proxy handler will run in user mode
                           We avoid saving data in registers because redirection to proxy handler is not known to the process
                           Thus, there is a substantial chance of data corruption if the kernel modifies any of the GPRs */
                        process->reg_context->sp0 -= 24;
                        int64_t* sp0 = (int64_t*)process->reg_context->sp0;
                        sp0[0] = i;
                        sp0[1] = (int64_t)process->handlers[i];
                        sp0[2] = el0_addr;
                    }
                    else{ /* Run default handler in kernel context */
                        target_proc = process;
                        process->handlers[i](i);
                    }
                }
                /* Clear the signal by XORing specific bit now that it is addressed and replace the handler with default */
                process->signals ^= (1 << i);
                process->handlers[i] = def_handlers[i];
            }
        }
    }
}

static void def_handler_entry(int signal)
{    
    switch (signal)
    {
    case SIGINT:
    case SIGABRT:
    case SIGTERM: { /* Graceful termination where orphans are reassigned, parent informed and memory cleaned */
        /* Remove the process from the ready queue */
        remove(&pc->ready_que, (struct Node*)target_proc);
        /* Handover orphan children if any to the init process */
        switch_parent(target_proc->pid, 1);
        /* Invoke exit for the process to unblock the parent if it is waiting */
        exit(target_proc, true);
        /* If the parent wasn't waiting or hasn't got to cleaning the child resources yet, perform the cleanup */
        if (target_proc->state != UNUSED){
            /* Release memory used by the process for page map and stack */
            kfree(target_proc->stack);
            free_uvm(target_proc->page_map);
            /* Decrement ref counts of all files left open by the process */
            for(int i = 0; i < MAX_OPEN_FILES; i++)
            {
                if (target_proc->fd_table[i] != NULL){
                    target_proc->fd_table[i]->ref_count--;
                    target_proc->fd_table[i]->inode->ref_count--;
                    /* Note that we can't release the inode based on only the file entry ref count because other file entries could be pointing to it
                       Hence the in core inode should be released (entry set to NULL) only if the inode ref count is zero */
                    if (target_proc->fd_table[i]->inode->ref_count == 0)
                        target_proc->fd_table[i]->inode = NULL;
                }
            }
            /* Set process state to unused and reset daemon status to avoid it getting carried over to another process using this slot */
            target_proc->state = UNUSED;
            target_proc->daemon = false;
        }
        break;
    }
    case SIGKILL: /* Abrupt and fast killing of a process where cleanup is not performed. May result in unattended zombies */
        /* Ignore kill request for idle and init process */
        if (target_proc->pid == 0 || target_proc->pid == 1)
            return;
        /* Remove the process from the ready queue */
        remove(&pc->ready_que, (struct Node*)target_proc);
        /* Yield the current foreground status if holding one, for other processes to claim */
        if (pc->fg_process != NULL){
            if (target_proc->pid == pc->fg_process->pid)
                pc->fg_process = NULL;
        }
        target_proc->state = KILLED;
        break;
    case SIGCHLD:
        /* SIGCHLD is not supposed to be handled by the kernel. It's the parent's responsibilty */
    default:
        break;
    }
}

void register_handler(struct Process* process, int signal, SIGHANDLER new_handler)
{
    /* Check the range and disallow custom handling for SIGKILL and SIGSTOP since they cannot be caught or ignored */
    if (signal > 0 && signal < TOTAL_SIGNALS-1 && !(signal == SIGKILL || signal == SIGSTOP)){
        if (new_handler == SIG_IGN)
            process->handlers[signal] = NULL;
        else if (new_handler == SIG_DFL)
            process->handlers[signal] = def_handler_entry;
        else
            process->handlers[signal] = new_handler;
    }
}

void set_sighandler_proxy(SIGHANDLER_PROXY handler)
{
    proxy_handler = handler;
}

void init_def_handlers(struct ProcessControl* proc_ctrl)
{
    pc = proc_ctrl;
    
    memset(def_handlers, 0, sizeof(SIGHANDLER)*TOTAL_SIGNALS);
    /* Note that the kernel will not host default handlers for all signals. If custom handler is missing, the signal may get lost */
    def_handlers[SIGINT] = def_handler_entry;
    def_handlers[SIGTERM] = def_handler_entry;
    def_handlers[SIGKILL] = def_handler_entry;
}

void init_handlers(struct Process *process)
{
    process->signals = 0;
    memcpy(process->handlers, def_handlers, sizeof(SIGHANDLER)*TOTAL_SIGNALS);
}
