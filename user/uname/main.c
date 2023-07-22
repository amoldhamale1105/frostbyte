#include "stdlib.h"

static void print_usage(void)
{
    printf("Usage:\n");
    printf("\tuname [OPTION]\n");
    printf("Print certain system information.  With no OPTION, same as -s.\n\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf("\t-a\tprint all system information in following order:\n");
    printf("\t-s\tprint the kernel name\n");
    printf("\t-r\tprint the kernel version\n");
    printf("\t-i\tprint the hardware platform architecture\n");
}

int main(int argc, char** argv)
{
    char option = 's';
    if (argc > 1){
        int arg_len = strlen(argv[1]);
        if (argv[1][0] != '-' || arg_len > 2){
            printf("%s: invalid option \'%s\'\n", argv[0], argv[1]);
            printf("Try \'%s -h\' for more information\n", argv[0]);
            return 1;
        }
        option = argv[1][1];
    }
    
    switch (option)
    {
    case 'h':
        print_usage();
        break;
    case 'r':
        printf("%s\n", stringify_value(VERSION));
        break;
    case 'a':
        printf("%s %s %s\n", stringify_value(NAME), stringify_value(VERSION), stringify_value(ARCH));
        break;
    case 'i':
        printf("%s\n", stringify_value(ARCH));
        break;
    case 's':
        printf("%s\n", stringify_value(NAME));
        break;
    default:
        printf("%s: invalid option \'%s\'\n", argv[0], argv[1]);
        printf("Try \'%s -h\' for more information\n", argv[0]);
        break;
    }
    
    return 0;
}