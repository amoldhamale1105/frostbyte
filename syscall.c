#include "syscall.h"
#include "print.h"
#include "uart.h"
#include "debug.h"
#include "stddef.h"

static SYSTEMCALL syscall_list[TOTAL_SYSCALL_FUNCTIONS];

static int sys_write(int64_t *arg)
{
    /* Pass the first argument on the stack which contains the pointer to the char array */
    write_string((char*)arg[0]);
    /* Return the count of characters printed to the console */
    return (int)arg[1];
}

void init_system_call(void)
{
    syscall_list[0] = sys_write;
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
    if (argc < 0 || index != 0){
        ctx->x0 = -1;
        return;
    }

    /* Call the system function associated with the index provided from the user program.
       Save the return value in register x0 position in the context frame on the stack */
    ctx->x0 = syscall_list[index](arg);
}
