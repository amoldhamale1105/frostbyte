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

#include "login.h"
#include "flib.h"

int getline(char* file_buf)
{
    if (file_buf == NULL || *file_buf == '\0')
        return -1;
    int newline_offset = 0;
    while (*(file_buf+newline_offset) != '\n')
    {
        if (*(file_buf+newline_offset) == '\0'){
            newline_offset = -1;
            break;
        }
        newline_offset++;
    }

    return newline_offset;
}

int read_passwd(char* buf)
{
    int buf_size = 0;
    char cmd_ch;

    if (!buf)
        return 0;
    while (1)
    {
        cmd_ch = getchar();

        /* Stop reading and exit when enter key (carriage return) is pressed by user */
        if (cmd_ch == '\r'){
            buf[buf_size] = 0;
            break;
        }
        else if (cmd_ch == ASCII_BACKSPACE){
            if (buf_size == 0)
                continue;
            /* Erase the last read character from the buffer */
            buf[--buf_size] = 0;
        }
        else{
            if (cmd_ch == ASCII_CTRL_C || cmd_ch == ASCII_ESCAPE)
                continue;
            buf[buf_size++] = cmd_ch;
        }
        /* End password entry if the max password size exceeded */
        if (buf_size >= MAX_PASSWD_SIZE-1){
            buf[MAX_PASSWD_SIZE-1] = 0;
            break;
        }
    }

    return buf_size;
}