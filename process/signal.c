/**
    Frostbyte kernel and operating system
    Copyright (C) 2023  Amol Dhamale <amoldhamale1105@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

void def_handler_entry(int signal);

void check_pending_signals(struct Process* process)
{
    if (process != NULL && process->signals > 0){
        for(int i = 0; i < TOTAL_SIGNALS; i++)
        {
            /* Whether the process is still alive to check for precence of other signals */
            if (process->state == UNUSED || process->state == KILLED)
                break;
            if (process->signals & (1 << i)){
                if (process->state == STOPPED && !(i == SIGKILL || i == SIGCONT))
                    continue;
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
                        /* Reset handler in process table entry to default */
                        process->handlers[i] = def_handlers[i];
                    }
                    else{ /* Run default handler in kernel context */
                        target_proc = process;
                        process->handlers[i](i);
                    }
                    /* A SIGCONT should always cause a process to be continued regardless of whether it is caught or ignored */
                    if (i == SIGCONT && !target_proc){
                        target_proc = process;
                        def_handler_entry(i);
                    }
                    target_proc = NULL;
                }
                /* Clear the signal by XORing specific bit, now that it is addressed */
                process->signals ^= (1 << i);
            }
        }
    }
}

void def_handler_entry(int signal)
{    
    switch (signal)
    {
    case SIGHUP:
    case SIGINT:
    case SIGABRT:
    case SIGTERM: { /* Graceful termination where orphans are reassigned, parent informed and memory cleaned */
        /* Remove the process from applicable active queue */
        if (contains(&pc->ready_que, (struct Node*)target_proc))
            remove(&pc->ready_que, (struct Node*)target_proc);
        else if (contains(&pc->wait_list, (struct Node*)target_proc))
            remove(&pc->wait_list, (struct Node*)target_proc);
        /* Invoke exit to do the rest */
        exit(target_proc, (1 << 8) | signal, true);
        break;
    }
    case SIGKILL: { /* Abrupt and forced killing of a process and its children. May result in rogue zombies */
        /* Ignore kill request for idle and init process */
        if (target_proc->pid == 0 || target_proc->pid == 1)
            return;
        target_proc->status = 1 << 8;
        target_proc->status |= signal & 0x7f;
        /* Inform the parent and pass the child's exit status */
        struct Process* parent = get_process(target_proc->ppid);
        if (parent != NULL && parent->state != KILLED){
            parent->signals |= (1 << SIGCHLD);
            parent->status = target_proc->status;
        }
        if (target_proc->state == STOPPED)
            remove(&pc->suspended, (struct Node*)target_proc);
        else{
            /* Remove the process from applicable active queue */
            if (contains(&pc->ready_que, (struct Node*)target_proc))
                remove(&pc->ready_que, (struct Node*)target_proc);
            else if (contains(&pc->wait_list, (struct Node*)target_proc))
                remove(&pc->wait_list, (struct Node*)target_proc);
            /* Yield the current foreground status if holding one, for other processes to claim */
            if (pc->fg_process != NULL){
                if (target_proc->pid == pc->fg_process->pid)
                    pc->fg_process = parent && !parent->daemon ? parent : NULL;
            }
            /* Wake up processes that might be paused while this one was running in the foreground */
            if (!target_proc->daemon)
                wake_up(FG_PAUSED);
        }
        /* Make init the new parent to clean up killed children */
        switch_parent(target_proc->pid, 1, false);
        kill(target_proc, 0, SIGKILL);
        /* Mark as killed and reset process table entry fields which might interfere with a new process occupying that slot */
        target_proc->state = KILLED;
        target_proc->event = target_proc->pid;
        target_proc->daemon = false;
        push_back(&pc->zombies, (struct Node*)target_proc);
        /* Unblock the parent if it is waiting */
        wake_up(STATE_CHANGE);
        break;
    }
    case SIGTSTP:
    case SIGSTOP: {
        /* Ignore stop request for idle and init process */
        if (target_proc->pid == 0 || target_proc->pid == 1)
            return;
        if (target_proc->state == STOPPED)
            return;
        /* Remove the process from applicable active queue */
        if (contains(&pc->ready_que, (struct Node*)target_proc))
            remove(&pc->ready_que, (struct Node*)target_proc);
        else if (contains(&pc->wait_list, (struct Node*)target_proc))
            remove(&pc->wait_list, (struct Node*)target_proc);
        target_proc->status |= 0x7f;
        /* Inform the parent and create job */
        struct Process* parent = get_process(target_proc->ppid);
        if (parent != NULL && parent->state != KILLED){
            parent->signals |= (1 << SIGCHLD);
            parent->status = target_proc->status;
            parent->wpid = target_proc->pid;
            if (!target_proc->job_spec){ /* Preserve job if already defined */
                parent->jobs++;
                target_proc->job_spec = parent->jobs;
            }
        }
        /* Yield the current foreground status if holding one, for other processes to claim */
        if (pc->fg_process != NULL){
            if (target_proc->pid == pc->fg_process->pid)
                pc->fg_process = parent && !parent->daemon ? parent : NULL;
        }
        /* Wake up processes that might be paused while this one was running in the foreground */
        if (!target_proc->daemon)
            wake_up(FG_PAUSED);
        target_proc->state = STOPPED;
        push_back(&pc->suspended, (struct Node*)target_proc);
        /* Unblock the parent if it is waiting */
        wake_up(STATE_CHANGE);
        break;
    }
    case SIGCONT: {
        /* Ignore continue request for idle and init process */
        if (target_proc->pid == 0 || target_proc->pid == 1)
            return;
        /* Remove process from the suspended list and place it on ready queue if it is currently stopped */
        if (target_proc->state == STOPPED){
            remove(&pc->suspended, (struct Node*)target_proc);
            /* Restore the process' state based on the event field */
            if (target_proc->event == NONE){
                target_proc->state = READY;
                push_back(&pc->ready_que, (struct Node*)target_proc);
            }
            else{
                target_proc->state = SLEEP;
                push_back(&pc->wait_list, (struct Node*)target_proc);
            }
            /* Pause the current foreground process if signal is being handled for a foreground process */
            if (!target_proc->daemon){
                if (pc->fg_process){
                    pc->fg_process->state = SLEEP;
                    pc->fg_process->event = FG_PAUSED;
                    if (contains(&pc->ready_que, (struct Node*)pc->fg_process))
                        remove(&pc->ready_que, (struct Node*)pc->fg_process);
                    push_back(&pc->wait_list, (struct Node*)pc->fg_process);
                }
                pc->fg_process = target_proc;
            }
            /* Clear previous stopped status and assign new status */
            target_proc->status &= ~0x7f;
            target_proc->status |= signal & 0x7f;
            /* Inform the parent */
            struct Process* parent = get_process(target_proc->ppid);
            if (parent != NULL && parent->state != KILLED)
                parent->signals |= (1 << SIGCHLD);
        }
        break;
    }
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
    def_handlers[SIGHUP] = def_handler_entry;
    def_handlers[SIGINT] = def_handler_entry;
    def_handlers[SIGABRT] = def_handler_entry;
    def_handlers[SIGTERM] = def_handler_entry;
    def_handlers[SIGKILL] = def_handler_entry;
    def_handlers[SIGSTOP] = def_handler_entry;
    def_handlers[SIGTSTP] = def_handler_entry;
    def_handlers[SIGCONT] = def_handler_entry;
}

void init_handlers(struct Process *process)
{
    process->signals = 0;
    memcpy(process->handlers, def_handlers, sizeof(SIGHANDLER)*TOTAL_SIGNALS);
}
