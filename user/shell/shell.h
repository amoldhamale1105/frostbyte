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

#ifndef SHELL_H
#define SHELL_H

#include "flib.h"
#include <stddef.h>
#include <stdbool.h>

#define MAX_CMD_BUF_SIZE 1024
#define MAX_PROG_ARGS 100

#define buf_offset(base, ptr) (int)((uint64_t)(ptr) - (uint64_t)(base))

extern bool interrupted; /* Whether an interrupt signal (SIGINT) was received by the shell */

int get_cmd_info(char* cmd, char* echo, int* cmd_pos, char** ext, char** echo_args);
int read_cmd(char* buf, char* echo_buf);

#endif