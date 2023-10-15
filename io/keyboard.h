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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* A circular buffer for key strokes */
struct KeyBuffer
{
    char buf[500];
    int front;  /* buffer read position */
    int end;    /* buffer write position */
};

#define MAX_KEY_BUF_SIZE 500

#define ASCII_CTRL_C 0x03
#define ASCII_CTRL_Z 26

char read_key_buffer(void);
void capture_key(void);
void notify_process(void);

#endif