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

#include "flib.h"
#include <stdbool.h>
#include <stddef.h>

#define MAX_SCAN_BUF_SIZE 1024

int strlen(const char* str)
{
    int len = 0;
    while (*(str + len) != '\0')
    {
        len++;
    }
    return len;
}

char to_upper(char ch)
{
    if (ch >= ASCII_LOWERCASE_START && ch <= ASCII_LOWERCASE_END)
        return (char)(ch - ASCII_UPPERCASE_OFFSET);
    return ch;
}

void to_upper_str(char *str)
{
    while (*str)
    {
        *str++ = to_upper(*str);
    }
}

int atoi(char *str)
{
    bool neg = false;
    int num = 0;
    
    if (*str == '-' || *str == '+'){
        if (*str == '-')
            neg = true;
        str++;
    }
    int digits = strlen(str);
    int order = digits-1;

    while (*str)
    {
        if (*str < BASE_NUMERIC_ASCII || *str > BASE_NUMERIC_ASCII+9)
            return 0;
        int digit = (int)*str - BASE_NUMERIC_ASCII;
        num += digit * power(10, order--);
        str++;
    }
    
    return neg ? -num : num;
}

uint32_t atoui(char *str)
{
    uint32_t num = 0;
    
    if (*str == '-' || *str == '+'){
        if (*str == '-')
            return 0;
        str++;
    }
    int digits = strlen(str);
    int order = digits-1;

    while (*str)
    {
        if (*str < BASE_NUMERIC_ASCII || *str > BASE_NUMERIC_ASCII+9)
            return 0;
        int digit = (int)*str - BASE_NUMERIC_ASCII;
        num += digit * power(10, order--);
        str++;
    }
    
    return num;
}

uint64_t atox(char *str)
{
    uint64_t num = 0;
    
    if (*str == '-' || *str == '+'){
        if (*str == '-')
            return 0;
        str++;
    }
    int digits = strlen(str);
    int order = digits-1;

    while (*str)
    {
        if (*str < BASE_NUMERIC_ASCII || *str > BASE_NUMERIC_ASCII+9)
            return 0;
        int digit = (int)*str - BASE_NUMERIC_ASCII;
        num += digit * power(10, order--);
        str++;
    }
    
    return num;
}

int64_t power(int base, int exp)
{
    if (exp == 0)
        return 1;
    
    int64_t half_pow = power(base, exp/2);
    int64_t pow = half_pow*half_pow;

    if (exp & 1)
        pow = base*pow;
    return pow;
}

int abs(int num)
{
    return num < 0 ? -num : num;
}

static int read_input(char* buf, int max_size)
{
    char shell_echo[4];
    int write_count = 0;

    memset(shell_echo, 0, sizeof(shell_echo));
    
    while (1)
    {
        if (write_count >= max_size-1){
            buf[max_size-1] = 0;
            break;
        }
        char ch = getchar();

        if (ch == '\r'){
            shell_echo[0] = '\r';
            shell_echo[1] = '\n';
            shell_echo[2] = 0;
            printf("%s", shell_echo);
            /* Null terminate the character echo buffer for upcoming character */
            shell_echo[1] = 0;
            buf[write_count] = 0;
            break;
        }
        else if (ch == ' '){
            *shell_echo = ch;
            printf("%s", shell_echo);
            /* Null terminate a word if it was being recorded and continue counting key strokes */
            if (write_count > 0)
                buf[write_count++] = 0;
        }
        else if (ch == ASCII_DELETE){
            if (write_count == 0)
                continue;
            /* Use a combination of backspace and space characters to clear the last typed character on the screen */
            shell_echo[0] = '\b';
            shell_echo[1] = ' ';
            shell_echo[2] = '\b';
            printf("%s", shell_echo);
            if (buf[write_count-1] == ASCII_ESCAPE)
                printf("%s", shell_echo);
            shell_echo[1] = 0;
            /* Erase the last read character from the buffer */
            buf[--write_count] = 0;
        }
        else if (ch == ASCII_ESCAPE){
            shell_echo[0] = '^';
            shell_echo[1] = '[';
            shell_echo[2] = 0;
            printf("%s", shell_echo);
            /* Null terminate the character echo buffer for upcoming character */
            shell_echo[1] = 0;
            buf[write_count++] = ch;
        }
        else{
            if (ch == ASCII_CTRL_C)
                continue;
            *shell_echo = ch;
            printf("%s", shell_echo);
            buf[write_count++] = ch;
        }
    }

    return write_count;
}

int scanf(const char *fmt, ...)
{
    va_list ap;
    const char* p;
    char buf[MAX_SCAN_BUF_SIZE] = {0};
    int written, pos = 0, assigned = 0;
    char* sptr;
    int* iptr;
    char* cptr;
    uint64_t* xptr;
    uint32_t* uptr;
    
    va_start(ap, fmt);
    written = read_input(buf, MAX_SCAN_BUF_SIZE);

    for(p = fmt; *p; p++)
    {
        if (*p != '%'){
            if (*p == '\r' || *p == '\n' || *p == '\t' || *p == ' ')
                continue;
            break;
        }
        if (pos >= written){
            memset(buf, 0, written);
            /* Persist until user provides a legit input. Ignore carriage returns without data */
            while (!(written = read_input(buf, MAX_SCAN_BUF_SIZE)));
            pos = 0;
        }

        switch (*++p)
        {
        case 'c':
            cptr = va_arg(ap, char*);
            if (cptr != NULL){
                *cptr = buf[pos];
                pos += 2;
                assigned++;
                continue;
            }
            break;
        case 's':
            sptr = va_arg(ap, char*);
            if (sptr != NULL){
                int slen = strlen(buf+pos);
                memcpy(sptr, buf+pos, slen);
                pos += (slen+1);
                assigned++;
                continue;
            }
            break;
        case 'd':
            iptr = va_arg(ap, int*);
            if (iptr != NULL){
                int slen = strlen(buf+pos);
                *iptr = atoi(buf+pos);
                pos += (slen+1);
                assigned++;
                continue;
            }
            break;
        case 'u':
            uptr = va_arg(ap, uint32_t*);
            if (uptr != NULL){
                int slen = strlen(buf+pos);
                *uptr = atoui(buf+pos);
                pos += (slen+1);
                assigned++;
                continue;
            }
            break;
        case 'x':
            xptr = va_arg(ap, uint64_t*);
            if (xptr != NULL){
                int slen = strlen(buf+pos);
                *xptr = atox(buf+pos);
                pos += (slen+1);
                assigned++;
                continue;
            }
            break;
        default:
            assigned = -1;
            break;
        }
        /* This is needed for a switch statement because a regular break won't exit the outer loop */
        break;
    }

    va_end(ap);
    return assigned;
}
