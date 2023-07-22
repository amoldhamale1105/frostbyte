#ifndef SHELL_H
#define SHELL_H

#include "flib.h"
#include <stddef.h>
#include <stdbool.h>

#define MAX_CMD_BUF_SIZE 1024
#define MAX_PROG_ARGS 100
#define ASCII_BACKSPACE 127
#define ASCII_ESCAPE 27
#define ASCII_CTRL_C 0x03

#define buf_offset(base, ptr) (int)((uint64_t)(ptr) - (uint64_t)(base))

extern bool interrupted; /* Whether an interrupt signal (SIGINT) was received by the shell */

int get_cmd_info(char* cmd, char* echo, int* cmd_pos, char** ext, char** echo_args);
int read_cmd(char* buf, char* echo_buf);

#endif