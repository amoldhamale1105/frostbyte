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

#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdint.h>

#define TOTAL_SIGNALS 32

#define SIGHUP  1
#define SIGINT  2
#define SIGABRT 6
#define SIGKILL 9
#define SIGTERM 15
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20 /* Stop signal issued from shell */

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