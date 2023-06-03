#include "syscall.h"
#include <io/print.h>
#include <io/uart.h>
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
    exit();
    return 0;
}

static int64_t sys_wait(int64_t* argv)
{
    wait(argv[0]);
    return 0;
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

    /* If not a valid syscall, return an error code -1 */
    if (argc < 0 || (index < 0 || index > TOTAL_SYSCALL_FUNCTIONS-1)){
        ctx->x0 = -1;
        return;
    }

    /* Call the system function associated with the index provided from the user program.
       Save the return value in register x0 position in the context frame on the stack */
    ctx->x0 = syscall_list[index](argv);
}
