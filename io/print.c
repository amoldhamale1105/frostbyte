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

#include "print.h"
#include <io/uart.h>

int printk(const char *fmt, ...)
{
    va_list ap;
    const char* p;
    char* sval;
    int ival;
    uint8_t cval;
    uint64_t xval;
    uint32_t uval;
    
    va_start(ap, fmt);
    for(p = fmt; *p; p++)
    {
        if (*p != '%')
        {
            if (*p == '\n')
                write_char('\r');
            write_char(*p);
            continue;
        }

        switch(*++p)
        {
        case 'c':
            cval = (uint8_t)va_arg(ap, int);
            write_char(cval);
            break;
        case 'x':
            xval = va_arg(ap, uint64_t);
            write_string(xtoa(xval));
            break;
        case 'd':
            ival = va_arg(ap, int);
            write_string(itoa(ival));
            break;
        case 's':
            sval = va_arg(ap, char*);
            write_string(sval);
            break;
        case 'u':
            uval = va_arg(ap, uint32_t);
            write_string(uitoa(uval));
            break;
        default:
            write_char(*p);
            break;
        }
    }

    return 0;
}

char *itoa(int dec_val)
{
    static char int_out[50];
    int count = 0;
    int neg = 0;
    
    if (dec_val < 0){
        dec_val = -dec_val;
        neg = 1;
    }
    
    do
    {
        *(int_out+neg+count++) = dec_val%10 + BASE_NUMERIC_ASCII;
        dec_val /= 10;
    } while (dec_val != 0);

    /* Reverse the buffer */
    for (int i = 0; i < count/2; i++)
    {
        char temp = int_out[i+neg];
        int_out[i+neg] = int_out[count-i-1+neg];
        int_out[count-i-1+neg] = temp;
    }
    int_out[count+neg] = 0;
    if (neg == 1)
        int_out[0] = '-';
    
    return int_out;
}

char *uitoa(uint32_t dec_val)
{
    static char uint_out[50];
    int count = 0;
    
    do
    {
        *(uint_out+count++) = dec_val%10 + BASE_NUMERIC_ASCII;
        dec_val /= 10;
    } while (dec_val != 0);

    /* Reverse the buffer */
    for (int i = 0; i < count/2; i++)
    {
        char temp = uint_out[i];
        uint_out[i] = uint_out[count-i-1];
        uint_out[count-i-1] = temp;
    }
    uint_out[count] = 0;
    
    return uint_out;
}

char *xtoa(uint64_t hex_val)
{
    static char hex_out[18];
    int count = 0;

    do
    {
        int digit = hex_val%16;
        *(hex_out+2+count++) = digit < 10 ? digit + BASE_NUMERIC_ASCII : digit%10 + BASE_CAPS_ALPHA_ASCII;
        hex_val /= 16;
    } while (hex_val != 0);

    /* Reverse the buffer */
    for (int i = 0; i < count/2; i++)
    {
        char temp = hex_out[i+2];
        hex_out[i+2] = hex_out[count-i+1];
        hex_out[count-i+1] = temp;
    }
    hex_out[0] = '0';
    hex_out[1] = 'x';
    hex_out[count+2] = 0;
    
    return hex_out;
}
