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

#include "syscall.h"
#include <io/print.h>
#include <io/uart.h>
#include <io/keyboard.h>
#include <debug/debug.h>
#include <stddef.h>
#include <process/process.h>
#include <fs/file.h>

static SYSTEMCALL syscall_list[TOTAL_SYSCALL_FUNCTIONS];

static int64_t sys_write(int64_t *argv)
{
    /* Pass the first argument on the stack which contains the pointer to the char array */
    write_string((char*)argv[0]);
    /* Return the count of characters printed to the console */
    return (int)argv[1];
}

static int64_t sys_sleep(int64_t* argv)
{
    uint64_t ticks = get_ticks();
    uint64_t sleep_ticks = argv[0];

    uint64_t target_ticks = ticks + sleep_ticks;

    /* Put the process to sleep at every tick until the sleep duration has elapsed */
    while (ticks < target_ticks)
    {
        sleep(SLEEP_SYSCALL);
        ticks = get_ticks();
    }

    return 0;
}

static int64_t sys_exit(int64_t* argv)
{
    exit(get_curr_process(), argv[0], false);
    return 0;
}

static int64_t sys_wait(int64_t* argv)
{
    return wait(argv[0], (int*)argv[1], argv[2]);
}

static int64_t sys_open_file(int64_t* argv)
{
    return open_file(get_curr_process(), (char*)argv[0]);
}

static int64_t sys_close_file(int64_t* argv)
{
    close_file(get_curr_process(), argv[0]);
    return 0;
}

static int64_t sys_file_size(int64_t* argv)
{
    return get_file_size(get_curr_process(), argv[0]);
}

static int64_t sys_read_file(int64_t* argv)
{
    return read_file(get_curr_process(), argv[0], (void*)argv[1], argv[2]);
}

static int64_t sys_fork(int64_t* argv)
{
    return fork();
}

static int64_t sys_exec(int64_t* argv)
{
    return exec(get_curr_process(), (char*)argv[0], (const char**)argv[1]);
}

static int64_t sys_keyboard_read(int64_t* argv)
{
    struct Process* curr_process = get_curr_process();
    /* If the process waiting for keyboard input is not a foreground process, put it to sleep */
    if (curr_process->daemon)
        sleep(DAEMON_INPUT);
    else{
        while (curr_process->pid != get_fg_process()->pid)
        {
            sleep(FG_PAUSED);
        }
    }
    return read_key_buffer();
}

static int64_t sys_get_pid(int64_t* argv)
{
    return get_curr_process()->pid;
}

static int64_t sys_read_root_dir(int64_t* argv)
{
    return read_root_dir_table((char*)argv[0]);
}

static int64_t sys_get_ppid(int64_t* argv)
{
    return get_curr_process()->ppid;
}

static int64_t sys_active_procs(int64_t* argv)
{
    int* pid_list = (int*)argv[0];
    return get_active_pids(pid_list);
}

static int64_t sys_pstatus(int64_t* argv)
{
    return get_status(get_curr_process()->ppid);
}

static int64_t sys_proc_data(int64_t* argv)
{
    return get_proc_data(argv[0], (int*)argv[1], (int*)argv[2], (char*)argv[3], (char*)argv[4]);
}

static int64_t sys_kill(int64_t* argv)
{
    /* Forbid signals to the init process */
    if (argv[0] == 1)
        return -1;
    return kill(argv[0], argv[1]);
}

static int64_t sys_signal(int64_t* argv)
{
    register_handler(get_curr_process(), argv[0], (SIGHANDLER)argv[1]);
    set_sighandler_proxy((SIGHANDLER_PROXY)argv[2]);
    return 0;
}

static void sigproxy_restore(struct ContextFrame *ctx)
{
    int64_t* sp0 = (int64_t*)ctx->sp0;
    /* Restore the overwritten values in registers x1 and x8, and previous EL0 program counter */
    ctx->x1 = sp0[0];
    ctx->x8 = sp0[1];
    ctx->elr = sp0[2];
    /* Reclaim space on the stack used to save proxy handler and proxy restore arguments */
    ctx->sp0 += 24;
}

void init_system_call(void)
{
    syscall_list[0] = sys_write;
    syscall_list[1] = sys_sleep;
    syscall_list[2] = sys_exit;
    syscall_list[3] = sys_wait;
    syscall_list[4] = sys_open_file;
    syscall_list[5] = sys_close_file;
    syscall_list[6] = sys_file_size;
    syscall_list[7] = sys_read_file;
    syscall_list[8] = sys_fork;
    syscall_list[9] = sys_exec;
    syscall_list[10] = sys_keyboard_read;
    syscall_list[11] = sys_get_pid;
    syscall_list[12] = sys_read_root_dir;
    syscall_list[13] = sys_get_ppid;
    syscall_list[14] = sys_active_procs;
    syscall_list[15] = sys_proc_data;
    syscall_list[16] = sys_kill;
    syscall_list[17] = sys_signal;
    syscall_list[18] = sys_pstatus;
}

void system_call(struct ContextFrame *ctx)
{
    /* Retrieve data from the stack in user mode */
    /* Get the index number of the systemcall from the stack */
    int64_t index = ctx->x8;
    /* Get the argument count from x0 on the stack */
    int64_t argc = ctx->x0;
    /* Get the pointer to arguments passed to the function */
    int64_t* argv = (int64_t*)ctx->x1;

    if (index == SIG_PROXY_REQUEST){
        sigproxy_restore(ctx);
        return;
    }
    /* If not a valid syscall, return an error code -1 */
    if (argc < 0 || (index < 0 || index > TOTAL_SYSCALL_FUNCTIONS-1)){
        ctx->x0 = -1;
        return;
    }

    /* Call the system function associated with the index provided from the user program.
       Save the return value in register x0 position in the context frame on the stack */
    ctx->x0 = syscall_list[index](argv);
}
