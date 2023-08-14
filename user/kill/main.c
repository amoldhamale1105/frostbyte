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
#include <stddef.h>

static void print_usage(void)
{
    printf("Usage:");
    printf("\tkill [OPTION] <pid>\n");
    printf("\tSend a signal to a process\n\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf("\t-sig\tsignal number (Refer to POSIX signals)\n");
}

int main(int argc, char** argv)
{
    int pid = -1;
    int signal = SIGTERM;
    if (argc < 2){
        printf("%s: bad usage\n", argv[0]);
        printf("Try \'%s -h\' for more information\n", argv[0]);
        return 1;
    }
    if (argc > 1){
        int opt = 1;
        while (opt < argc)
        {
            if (argv[opt][0] != '-' && (argv[opt][0] >= BASE_NUMERIC_ASCII && argv[opt][0] <= BASE_NUMERIC_ASCII+9)){
                pid = atoi(argv[opt]);
                opt++;
                continue;
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
                    if (argv[opt][1] > BASE_NUMERIC_ASCII && argv[opt][1] <= BASE_NUMERIC_ASCII+9){
                        signal = atoi(&argv[opt][1]);
                        break;
                    }
                    printf("%s: invalid option \'%s\'\n", argv[0], argv[opt]);
                    printf("Try \'%s -h\' for more information\n", argv[0]);
                    return 1;
                }
                optstr++;
            }
            opt++;
        }
        if (pid == -1){
            printf("%s: bad usage\n", argv[0]);
            printf("Try \'%s -h\' for more information\n", argv[0]);
            return 1;
        }
    }
    
    int state = -1;
    get_proc_data(pid, NULL, &state, NULL, NULL);
    if (pid <= 0 || pid == getpid() || state <= UNUSED){
        printf("%s: (%d) - No such process\n", argv[0], pid);
        return 1;
    }
    if (signal > SIGUNUSED){
        printf("%s: %d: invalid signal specification\n", argv[0], signal);
        return 1;
    }

    int ret = kill(pid, signal);
    if (ret < 0)
        printf("%s: Failed to send signal %d to PID %d\n", argv[0], signal, pid);
    
    return 0;
}