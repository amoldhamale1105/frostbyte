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

#ifndef FLIB_H
#define FLIB_H

#define BASE_NUMERIC_ASCII 48
#define BASE_CAPS_ALPHA_ASCII 65
#define stringify(sequence) #sequence
#define stringify_value(val) stringify(val)

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

struct DirEntry {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_ms;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t attr_index;
    uint16_t m_time;
    uint16_t m_date;
    uint16_t cluster_index;
    uint32_t file_size;
} __attribute__((packed));

enum En_ProcessState
{
    UNUSED = 0,
    INIT,
    RUNNING,
    READY,
    SLEEP,
    STOPPED,
    KILLED
};

#define ENTRY_AVAILABLE 0
#define ENTRY_DELETED 0xe5
#define ATTR_VOLUME_LABEL 0x08
#define ATTR_FILETYPE_DIRECTORY 0x10
#define ATTR_LONG_FILENAME 0x0f

#define MAX_FILENAME_BYTES 8
#define MAX_EXTNAME_BYTES 3

#define ASCII_LOWERCASE_START 97
#define ASCII_LOWERCASE_END 122
#define ASCII_UPPERCASE_OFFSET 32
#define ASCII_DELETE 127
#define ASCII_ESCAPE 27
#define ASCII_CTRL_C 0x03
#define ASCII_CTRL_Z 26

int printf(const char* fmt, ...);
int scanf(const char *fmt, ...);
char* itoa(int);
char* uitoa(uint32_t);
char* xtoa(uint64_t);

void memset(void* dst, int value, unsigned int size);
void memcpy(void* dst, void* src, unsigned int size);
void memmove(void* dst, void* src, unsigned int size);
int memcmp(void* src1, void* src2, unsigned int size);
int strlen(const char* str);
char to_upper(char ch);
char to_lower(char ch);
void to_upper_str(char* str);
int atoi(char* str);
uint32_t atoui(char* str);
uint64_t atox(char* str);
int64_t power(int base, int exp);
int abs(int num);
void sort(int* arr, size_t size);

/* System call library functions */

int writeu(char* buf, int buf_size);
void msleep(uint64_t ticks_10ms);
int open_file(char* filename);
int close_file(int fd);
uint32_t get_file_size(int fd);
uint32_t read_file(int fd, void* buffer, uint32_t size);
int fork(void);
int wait(int* wstatus);
int waitpid(int pid, int* wstatus, int options);
int exec(char* prog_file, const char* args[]);
void exit(int status);
int kill(int pid, int signum);
void signal(int signum, void (*handler)(int));
char getchar(void);
int getpid(void);
int getppid(void);
int get_pstatus(void);
int get_proc_data(int pid, int* ppid, int* state, int* job_spec, char* procname, char* procargs);
int read_root_dir(void* buf);
int get_active_procs(int* pid_list, int all);
int setjobctl(int job_spec, int req);
int getjpid(int job_spec);
int setenv(const char *name, const char *value, int overwrite);
char *getenv(const char *name);
int unsetenv(const char *name);
int getfullenv(char** list);
void switchpenv(void);

#endif