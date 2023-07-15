#ifndef SYSCALL_H
#define SYSCALL_H

#include <irq/handler.h>

typedef int64_t (*SYSTEMCALL)(int64_t *argv);
void init_system_call(void);
void system_call(struct ContextFrame* ctx);

#define TOTAL_SYSCALL_FUNCTIONS 18

/* Special request codes. DO NOT map these to regular syscall numbers */
#define SIG_PROXY_REQUEST       101

#endif