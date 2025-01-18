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

#include "shell.h"
#include <stdbool.h>

bool interrupted = false;

int get_cmd_info(char* cmd, char* echo, int* cmd_pos, char** ext, char** echo_args)
{
    char* cmd_ptr = NULL;
    char* echo_ptr = NULL;
    char* arg_ptr = NULL;
    bool new_arg = false;
    int arg_count = 0;

    if (cmd == NULL || echo == NULL || cmd_pos == NULL || ext == NULL || echo_args == NULL)
        return -1;
    
    *cmd_pos = 0;
    *ext = NULL;
    /* Check if there are any leading spaces to adjust command start position */
    while (cmd[*cmd_pos] == ' ')
    {
        (*cmd_pos)++;
    }
    cmd_ptr = cmd + *cmd_pos;
    arg_ptr = cmd_ptr;
    /* Traverse until first space which marks end of command and beginning of args if any */
    while (*arg_ptr != ' ' && *arg_ptr != '\0')
    {
        arg_ptr++;
    }
    *arg_ptr = 0;
    *ext = arg_ptr;
    /* NOTE Reverse searching is needed because filename can contain dots but the last dot will always signify extension separator */
    while (*ext != cmd_ptr && *(*ext-1) != '.')
    {
        (*ext)--;
    }
    if (*ext == cmd_ptr)
        *ext = NULL;
    /* Null terminate args in the echo buffer and append their addresses to the argument list */
    echo_ptr = echo + *cmd_pos + buf_offset(cmd_ptr, arg_ptr);
    *echo_ptr++ = 0;
    while (*echo_ptr != '\0')
    {
        if (*echo_ptr == ' '){
            if (new_arg){
                *echo_ptr = 0;
                new_arg = false;
            }
        }
        else{
            if (!new_arg){
                new_arg = true;
                echo_args[arg_count++] = echo_ptr;
            }
        }
        echo_ptr++;
    }
    /* Null terminate the array of arguments */
    echo_args[arg_count] = NULL;
    return arg_count;
}

int read_cmd(char* buf, char* echo_buf)
{
    char shell_echo[4];
    char key_acc[3];
    int buf_size = 0;
    int cursor = 0;
    char cmd_ch;

    memset(shell_echo, 0, sizeof(shell_echo));
    memset(key_acc, 0, sizeof(key_acc));

    while (1)
    {
        cmd_ch = getchar();
        if (interrupted){
            interrupted = false;
            /* Flush the command buffer and reset cursor position */
            memset(buf, 0, buf_size);
            memset(echo_buf, 0, buf_size);
            buf_size = 0;
            cursor = 0;
        }

        /* Stop reading and exit when enter key (carriage return) is pressed by user */
        if (cmd_ch == '\r'){
            shell_echo[0] = '\r';
            shell_echo[1] = '\n';
            shell_echo[2] = 0;
            printf("%s", shell_echo);
            /* Null terminate the character echo buffer for upcoming character */
            shell_echo[1] = 0;
            break;
        }
        else if (cmd_ch == ASCII_DELETE){ /* "Backspace" key pressed */
            if (*key_acc)
                memset(key_acc, 0, sizeof(key_acc));
            if (cursor == 0)
                continue;
            /* Use a combination of backspace and space characters to clear the last typed character on the screen */
            shell_echo[0] = '\b';
            shell_echo[1] = ' ';
            shell_echo[2] = '\b';
            printf("%s", shell_echo);
            if (cursor < buf_size){
                /* The last character should be erased from the screen before shifting left */
                shell_echo[0] = ' ';
                shell_echo[1] = '\b';
                shell_echo[2] = 0;
                printf("%s%s", echo_buf+cursor, shell_echo);
                int split_len = strlen(echo_buf+cursor);
                /* Reset the cursor position on screen to last erased character */
                for(int i = 0; i < split_len; i++)
                {
                    printf("\b");
                }
                /* Shift left to fill up space created by erased character */
                memmove(buf+cursor-1, buf+cursor, split_len);
                memmove(echo_buf+cursor-1, echo_buf+cursor, split_len);
            }
            shell_echo[1] = 0;
            /* Erase the last character from the buffer */
            buf[buf_size-1] = 0;
            echo_buf[buf_size-1] = 0;
            buf_size--;
            cursor--;
        }
        else if (cmd_ch == ASCII_ESCAPE){ /* Esc key or a special key sequence */
            *key_acc = cmd_ch;
        }
        else{
            if (cmd_ch == ASCII_CTRL_C || cmd_ch == ASCII_CTRL_Z)
                continue;
            if (*key_acc == ASCII_ESCAPE){
                int i = 1;
                for (; (i < sizeof(key_acc)) && *(key_acc + i); i++);
                if (i < sizeof(key_acc)){
                    key_acc[i] = cmd_ch;
                    if (i < sizeof(key_acc)-1)
                        continue;
                }
                if (key_acc[1] != '['){ /* Unknown sequence */
                    memset(key_acc, 0, sizeof(key_acc));
                    continue;
                }
                bool echo = false;
                switch (key_acc[2])
                {
                case 'A': /* Up arrow key */
                    /* This is where history list traversing logic can be added in future if desired. For now, do nothing */
                    break;
                case 'B': /* Down arrow key */
                    /* Do nothing */
                    break;
                case 'C': /* Right arrow key */
                    if (cursor < buf_size){
                        cursor++;
                        echo = true;
                    }
                    break;
                case 'D': /* Left arrow key */
                    if (cursor > 0){
                        cursor--;
                        echo = true;
                    }
                default:
                    break;
                }
                if (echo){
                    memcpy(shell_echo, key_acc, sizeof(key_acc));
                    printf("%s", shell_echo);
                    shell_echo[1] = 0;
                }
                memset(key_acc, 0, sizeof(key_acc));
                continue;
            }
            *shell_echo = cmd_ch;
            printf("%s", shell_echo);
            if (cursor < buf_size){
                printf("%s", echo_buf+cursor);
                int split_len = strlen(echo_buf+cursor);
                /* Reset the cursor position on screen to after the last written character */
                for(int i = 0; i < split_len; i++)
                {
                    printf("\b");
                }
                /* Shift right by single character to make room for the new character */
                memmove(buf+cursor+1, buf+cursor, split_len);
                memmove(echo_buf+cursor+1, echo_buf+cursor, split_len);
            }
            /* Insert the new character at cursor position */
            buf[cursor] = to_upper(cmd_ch);
            echo_buf[cursor] = cmd_ch;
            buf_size++;
            cursor++;
        }
    }

    return buf_size;
}