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
                        /* Reset handler in process table entry to default */
                        process->handlers[i] = def_handlers[i];
                    }
                    else{ /* Run default handler in kernel context */
                        target_proc = process;
                        process->handlers[i](i);
                    }
                }
                /* Clear the signal by XORing specific bit, now that it is addressed */
                process->signals ^= (1 << i);
            }
        }
    }
}

static void def_handler_entry(int signal)
{    
    switch (signal)
    {
    case SIGHUP:
    case SIGINT:
    case SIGABRT:
    case SIGTERM: { /* Graceful termination where orphans are reassigned, parent informed and memory cleaned */
        /* Remove the process from the ready queue */
        remove(&pc->ready_que, (struct Node*)target_proc);
        /* Invoke exit for the process to unblock the parent if it is waiting */
        exit(target_proc, signal, true);
        break;
    }
    case SIGKILL: /* Abrupt and fast killing of a process where cleanup is not performed. May result in unattended zombies */
        /* Ignore kill request for idle and init process */
        if (target_proc->pid == 0 || target_proc->pid == 1)
            return;
        target_proc->status |= signal & 0x7f;
        /* Remove the process from the ready queue and handover children if any to the init process */
        remove(&pc->ready_que, (struct Node*)target_proc);
        switch_parent(target_proc->pid, 1);
        /* Inform the parent */
        struct Process* parent = get_process(target_proc->ppid);
        if (parent != NULL)
            parent->signals |= (1 << SIGCHLD);
        /* Yield the current foreground status if holding one, for other processes to claim */
        if (pc->fg_process != NULL){
            if (target_proc->pid == pc->fg_process->pid)
                pc->fg_process = NULL;
        }
        /* Wake up processes that might be paused while this one was running in the foreground */
        if (!target_proc->daemon)
            wake_up(FG_PAUSED);
        target_proc->state = KILLED;
        push_back(&pc->zombies, (struct Node*)target_proc);
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
    def_handlers[SIGHUP] = def_handler_entry;
    def_handlers[SIGINT] = def_handler_entry;
    def_handlers[SIGABRT] = def_handler_entry;
    def_handlers[SIGTERM] = def_handler_entry;
    def_handlers[SIGKILL] = def_handler_entry;
}

void init_handlers(struct Process *process)
{
    process->signals = 0;
    memcpy(process->handlers, def_handlers, sizeof(SIGHANDLER)*TOTAL_SIGNALS);
}
