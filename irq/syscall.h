#ifndef SYSCALL_H
#define SYSCALL_H

#include <irq/handler.h>

typedef int64_t (*SYSTEMCALL)(int64_t *argv);
void init_system_call(void);
void system_call(struct ContextFrame* ctx);

#define TOTAL_SYSCALL_FUNCTIONS 11

#endif