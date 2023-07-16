#include "stdlib.h"

int main(int argc, char** argv)
{
    char option = 0;
    if (argc > 1){
        int arg_len = strlen(argv[1]);
        if (memcmp("-a", argv[1], arg_len) == 0)
            option = 'a';
        else if (memcmp("-r", argv[1], arg_len) == 0)
            option = 'r';
        else{
            printf("%s: invalid option \'%s\'\n", argv[0], argv[1]);
            return 1;
        }
    }
    
    switch (option)
    {
    case 'r':
        printf("%s\n", stringify_value(VERSION));
        break;
    case 'a':
        printf("%s %s %s\n", stringify_value(NAME), stringify_value(VERSION), stringify_value(ARCH));
        break;
    default:
        printf("%s\n", stringify_value(NAME));
        break;
    }
    
    return 0;
}