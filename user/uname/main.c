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