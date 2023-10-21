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
    printf("Usage:");
    printf("\tenv [OPTION]\n");
    printf("\tPrint current shell environment as name and value pairs\n\n");
    printf("\t-h\tdisplay this help and exit\n");
}

int main(int argc, char** argv)
{
    if (argc > 1){
        int opt = 1;
        while (opt < argc)
        {
            if (argv[opt][0] != '-'){
                printf("%s: bad usage\n", argv[0]);
                printf("Try \'%s -h\' for more information\n", argv[0]);
                return 1;
            }
            char* optstr = &argv[opt][1];
            while (*optstr)
            {
                switch (*optstr)
                {
                case 'h':
                    print_usage();
                    return 0;
                default:
                    printf("%s: invalid option \'%s\'\n", argv[0], argv[opt]);
                    printf("Try \'%s -h\' for more information\n", argv[0]);
                    return 1;
                }
                optstr++;
            }
            opt++;
        }
    }

    switchpenv();
    int env_count = getfullenv(NULL);
    if (env_count > 0){
        char* env[env_count];
        getfullenv(env);
        while (env_count--)
        {
            printf("%s=%s\n", env[env_count], getenv(env[env_count]));
        }
    }
    
    return 0;
}