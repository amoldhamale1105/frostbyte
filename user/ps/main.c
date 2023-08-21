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
#include <stddef.h>
#include <stdbool.h>

static char state_rep(int state)
{
    char state_ch;

    switch (state)
    {
    case INIT:
        state_ch = 'i';
        break;
    case RUNNING:
        state_ch = 'R';
        break;
    case READY:
        state_ch = 'r';
        break;
    case SLEEP:
        state_ch = 's';
        break;
    case STOPPED:
        state_ch = 'T';
        break;
    case KILLED:
        state_ch = 'z';
        break;
    default:
        state_ch = 0;
        break;
    }

    return state_ch;
}

static void print_usage(void)
{
    printf("Usage:");
    printf("\tps [OPTION...]\n");
    printf("\tReport a snapshot of current processes\n\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf("\t-e\tSelect all processes.  Identical to -A.\n");
    printf("\t-f\tfull format listing with additional columns and\n\t\tcommand arguments\n");
    printf("\t-A\tSelect all processes.  Identical to -e.\n");
    printf("\t-rows\trows is number of lines to display from the head\n");
}

int main(int argc, char** argv)
{
    int rows = 0;
    bool full_format = false;
    bool all = false;
    if (argc > 1){
        int opt = 1;
        while (opt < argc)
        {
            if (argv[opt][0] != '-'){
                printf("%s: bad usage\n", argv[0]);
                printf("Try \'%s -h\' for more information\n", argv[0]);
                return 1;
            }
            if (argv[opt][1] > BASE_NUMERIC_ASCII && argv[opt][1] <= BASE_NUMERIC_ASCII+9){
                rows = atoi(&argv[opt][1]);
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
                case 'e':
                case 'A':
                    all = true;
                    break;
                case 'f':
                    full_format = true;
                    break; /* NOTE This breaks from the switch block, not the outer loop */
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

    const char* ff_header = "PID    PPID    STATE    CMD";
    const char* sf_header = "PID    CMD";
    const char* header = full_format ? ff_header : sf_header;
    int header_len = strlen(header);
    char separator[header_len+2];
    for(int i = 0; i <= header_len; i++)
    {
        separator[i] = '-';
    }
    separator[header_len+1] = 0;
    
    int pid_count = get_active_procs(NULL, all);
    int pid_list[pid_count];
    if (rows == 0 || rows > pid_count)
        rows = pid_count;
    get_active_procs(pid_list, all);
    sort(pid_list, pid_count);

    int ppid, state, args_size, args_pos;
    char procname[MAX_FILENAME_BYTES+1];

    printf("%s\n", header);
    printf("%s\n", separator);
    for(int i = 0; i < rows; i++)
    {
        memset(procname, 0, sizeof(procname));
        args_size = get_proc_data(pid_list[i], NULL, NULL, NULL, procname, NULL);
        if (full_format){
            char procargs[args_size];
            get_proc_data(pid_list[i], &ppid, &state, NULL, NULL, args_size > 0 ? procargs : NULL);
            printf("%d\t%d\t%c\t%s ", pid_list[i], ppid, state_rep(state), procname);
            args_pos = 0;
            /* Print the process arguments from the procargs buffer filled by the kernel */
            while (args_pos < args_size)
            {
                printf("%s ", procargs+args_pos);
                args_pos += (strlen(procargs+args_pos)+1);
            }
            printf("\n");
            continue;
        }
        printf("%d\t%s\n", pid_list[i], procname);
    }
    
    return 0;
}