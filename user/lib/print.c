#include "stddef.h"
#include "stdlib.h"

int printf(const char *fmt, ...)
{
    va_list ap;
    const char* p;
    char* sval;
    int ival;
    uint64_t xval;
    uint32_t uval;
    char buf[UINT16_MAX];
    uint16_t count;
    
    va_start(ap, fmt);
    for(p = fmt, count = 0; *p; p++)
    {
        if (*p != '%')
        {
            buf[count++] = *p;
            continue;
        }

        char* fmt_spec_str;

        switch(*++p)
        {
        case 'x':
            xval = va_arg(ap, uint64_t);
            fmt_spec_str = xtoa(xval);
            break;
        case 'd':
            ival = va_arg(ap, int);
            fmt_spec_str = itoa(ival);
            break;
        case 's':
            sval = va_arg(ap, char*);
            fmt_spec_str = sval;
            break;
        case 'u':
            uval = va_arg(ap, uint32_t);
            fmt_spec_str = uitoa(uval);
            break;
        default:
            fmt_spec_str = NULL;
            break;
        }

        if (fmt_spec_str != NULL){
            do
            {
                buf[count++] = *fmt_spec_str++;
            } while (*fmt_spec_str);
        }
        else
            buf[count++] = *p;
    }

    buf[count] = 0;
    /* Invoke a system call to write the buffer to the console */
    count = writeu(buf, count);
    va_end(ap);
    return count;
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