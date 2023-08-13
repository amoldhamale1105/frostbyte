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
    int status; /* Exit status of the process */
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
#define WNOHANG 1

enum En_SleepEvent
{
    NONE = -255,
    SLEEP_SYSCALL,
    ZOMBIE_CLEANUP,
    KEYBOARD_INPUT,
    DAEMON_INPUT,
    FG_PAUSED
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
int get_status(int pid);
int get_proc_data(int pid, int* ppid, int* state, char* name, char* args_buf);
int get_active_pids(int* pid_list);
void switch_parent(int curr_ppid, int new_ppid);
void sleep(int event);
void wake_up(int event);
void exit(struct Process* process, int status, bool sig_handler_req);
int wait(int pid, int* wstatus, int options);
int fork(void);
int exec(struct Process* process, char* name, const char* args[]);
int kill(int pid, int signal);

#endif