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

#include "keyboard.h"
#include "uart.h"
#include "print.h"
#include <process/process.h>

static struct KeyBuffer key_buf = {
    .buf = {0},
    .front = 0,
    .end = 0,
};

static void write_key_buffer(char ch)
{
    int next_write_pos = (key_buf.end + 1) % MAX_KEY_BUF_SIZE;

    /* Check if the buffer is full */
    if (next_write_pos == key_buf.front)
        return;

    key_buf.buf[key_buf.end] = ch;
    key_buf.end = next_write_pos;
}

char read_key_buffer(void)
{
    /* If buffer is empty, sleep on keyboard input event */
    if (key_buf.front == key_buf.end)
        sleep(KEYBOARD_INPUT);

    char ch = key_buf.buf[key_buf.front];
    key_buf.front = (key_buf.front + 1) % MAX_KEY_BUF_SIZE;
    
    return ch;
}

void capture_key(void)
{
    /* stdin should only work for current foreground process */
    struct Process* fg_proc = get_fg_process();
    if (fg_proc == NULL)
        return;
    /* Capture the pressed key from uart module and check for special characters */
    char key = read_char();
    switch (key)
    {
    case ASCII_CTRL_C:
        kill(get_curr_process(), fg_proc->pid, SIGINT);
        break;
    case ASCII_CTRL_Z:
        kill(get_curr_process(), fg_proc->pid, SIGTSTP);
        break;
    default:
        /* Flush to stdout if foreground process not using key input to prevent residual characters in key buffer */
        if (fg_proc->event != KEYBOARD_INPUT){
            if (key == '\r')
                key = '\n';
            write_char(key);
            return;
        }
        break;
    }
    /* Push the key to circular buffer */
    write_key_buffer(key);
}

void notify_process(void)
{
    wake_up(KEYBOARD_INPUT);
}
