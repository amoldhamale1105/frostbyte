#include "syscall.h"
#include "print.h"
#include "uart.h"
#include "debug.h"
#include "stddef.h"
#include "handler.h"
#include "process.h"

static SYSTEMCALL syscall_list[TOTAL_SYSCALL_FUNCTIONS];

static int sys_write(int64_t *arg)
{
    /* Pass the first argument on the stack which contains the pointer to the char array */
    write_string((char*)arg[0]);
    /* Return the count of characters printed to the console */
    return (int)arg[1];
}

static int sys_sleep(int64_t* arg)
{
    uint64_t ticks = get_ticks();
    uint64_t sleep_ticks = arg[0];

    uint64_t target_ticks = ticks + sleep_ticks;

    /* Put the process to sleep at every tick until the sleep duration has elapsed */
    while (ticks < target_ticks)
    {
        sleep(SLEEP_SYSCALL);
        ticks = get_ticks();
    }

    return 0;
}

static int sys_exit(int64_t* arg)
{
    exit();
    return 0;
}

static int sys_wait(int64_t* arg)
{
    wait(arg[0]);
    return 0;
}

void init_system_call(void)
{
    syscall_list[0] = sys_write;
    syscall_list[1] = sys_sleep;
    syscall_list[2] = sys_exit;
    syscall_list[3] = sys_wait;
}

void system_call(struct ContextFrame *ctx)
{
    /* Retrieve data from the stack in user mode */
    /* Get the index number of the systemcall from the stack */
    int64_t index = ctx->x8;
    /* Get the argument count from x0 on the stack */
    int64_t argc = ctx->x0;
    /* Get the pointer to arguments passed to the function */
    int64_t* arg = (int64_t*)ctx->x1;

    /* If not a valid syscall, return an error code -1 */
    if (argc < 0 || (index < 0 || index > TOTAL_SYSCALL_FUNCTIONS-1)){
        ctx->x0 = -1;
        return;
    }

    /* Call the system function associated with the index provided from the user program.
       Save the return value in register x0 position in the context frame on the stack */
    ctx->x0 = syscall_list[index](arg);
}
