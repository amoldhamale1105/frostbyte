#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdint.h>

#define SIGHUP      1
#define SIGINT      2
#define SIGKILL     9
#define SIGTERM     15
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGUNUSED   31

#define SIG_DFL ((void (*)(int))0)     /* default signal handling */
#define SIG_IGN ((void (*)(int))1)     /* ignore signal */

#endif