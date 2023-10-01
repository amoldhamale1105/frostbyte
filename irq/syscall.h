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

#ifndef SYSCALL_H
#define SYSCALL_H

#include <irq/handler.h>

typedef int64_t (*SYSTEMCALL)(int64_t *argv);
void init_system_call(void);
void system_call(struct ContextFrame* ctx);

#define TOTAL_SYSCALL_FUNCTIONS 24

/* Special request codes. DO NOT map these to regular syscall numbers */
#define SIG_PROXY_REQUEST       101

#endif