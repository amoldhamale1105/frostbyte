#include "stdlib.h"
#include "stdbool.h"

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
