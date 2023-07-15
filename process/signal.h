#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdint.h>

#define TOTAL_SIGNALS 32

#define SIGHUP  1
#define SIGINT  2
#define SIGKILL 9
#define SIGTERM 15
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19

typedef void (*SIGHANDLER)(int);
typedef void (*SIGHANDLER_PROXY)(void);

#define SIG_DFL ((SIGHANDLER)0)     /* default signal handling */
#define SIG_IGN ((SIGHANDLER)1)     /* ignore signal */

struct Process;
struct ProcessControl;

void init_def_handlers(struct ProcessControl* proc_ctrl);
void init_handlers(struct Process* process);
void check_pending_signals(struct Process* process);
void register_handler(struct Process* process, int signal, SIGHANDLER new_handler);
void set_sighandler_proxy(SIGHANDLER_PROXY handler);

#endif