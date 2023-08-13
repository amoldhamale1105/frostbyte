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
#include "signal.h"
#include <sys/wait.h>

static void print_usage(void)
{
    printf("Usage:\n");
    printf("\texit [OPTION] [N]\n");
    printf("Exit the shell\n");
    printf("Exits the shell with a status of N. If N is omitted, the exit status\nis that of the last command executed.\n\n");
    printf("\t-h\tdisplay this help and exit\n");
}

int main(int argc, char** argv)
{
    int status = INT32_MAX;
    if (argc > 1){
        if (argv[1][0] == '-'){
            if (strlen(argv[1]) == 2){
                switch (argv[1][1])
                {
                case 'h':
                    print_usage();
                    return 0;
                default:
                    printf("%s: invalid option \'%s\'\n", argv[0], argv[1]);
                    printf("Try \'%s -h\' for more information\n", argv[0]);
                    return 1;
                }
            }
            else{
                printf("%s: bad usage\n", argv[0]);
                printf("Try \'%s -h\' for more information\n", argv[0]);
                return 1;
            }
        }
        status = atoi(argv[1]);
    }
    
    int shell_pid = getppid();
    if (kill(shell_pid, SIGTERM) < 0)
        return -1;
    
    return status == INT32_MAX ? WEXITSTATUS(get_pstatus()) : status;
}