#include "stdlib.h"

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
