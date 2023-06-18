#ifndef SHELL_H
#define SHELL_H

#include "stdlib.h"
#include "stddef.h"

#define MAX_CMD_BUF_SIZE 1024
#define MAX_PROG_ARGS 100
#define ASCII_BACKSPACE 127
#define ASCII_ESCAPE 27

#define buf_offset(base, ptr) (int)((uint64_t)(ptr) - (uint64_t)(base))

void get_cmd_info(char* cmd, char* echo, int* cmd_pos, char** ext, char** echo_args);
int read_cmd(char* buf, char* echo_buf);

#endif