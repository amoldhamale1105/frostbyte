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
#include <stdbool.h>
#include <stddef.h>

static void print_usage(void)
{
    printf("Usage:");
    printf("\tjobs [OPTION...] [jobspec]\n");
    printf("\tDisplay status of jobs.\n\n");
    printf("\tList the active jobs. JOBSPEC restricts output to that job.\n\tWithout options, the status of all active jobs is displayed.\n\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf("\t-l\tlists process IDs in addition to the normal information\n");
    printf("\t-p\tlists process IDs only\n");
    printf("\t-r\trestrict output to running jobs\n");
    printf("\t-s\trestrict output to stopped jobs\n");
}

void display_job(int job_spec, int pid, char* name, char* args, int args_size, bool show_pid, bool running)
{
    int args_pos = 0;
    
    if (show_pid)
        printf("[%d] %d\t%s\t%s ", job_spec, pid, running ? "Running" : "Stopped", name);
    else
        printf("[%d]  %s\t%s ", job_spec, running ? "Running" : "Stopped", name);
    
    while (args_pos < args_size)
    {
        printf("%s ", args+args_pos);
        args_pos += (strlen(args+args_pos)+1);
    }
    printf("%s\n", running ? "&" : " \b");
}

int main(int argc, char** argv)
{
    int req_js = -1;
    bool show_pid, show_but_pid = true;
    bool show_running = true, show_stopped = true;
    if (argc > 1){
        int opt = 1;
        while (opt < argc)
        {
            if (argv[opt][0] != '-' && (argv[opt][0] >= BASE_NUMERIC_ASCII && argv[opt][0] <= BASE_NUMERIC_ASCII+9)){
                req_js = atoi(argv[opt]);
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
                case 'p':
                    show_but_pid = false;
                case 'l':
                    show_pid = true;
                    break;
                case 'r':
                    show_running = true;
                    show_stopped = false;
                    break;
                case 's':
                    show_stopped = true;
                    show_running = false;
                    break;
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
    int pid_count = get_active_procs(NULL, false);
    int pid_list[pid_count];
    if (pid_count > 0){
        get_active_procs(pid_list, false);
        sort(pid_list, pid_count);
    }
    else{
        if (req_js != -1){
            printf("%s: %d: no such job\n", argv[0], req_js);
            return 1;
        }
        return 0;
    }
    
    int job_spec, state;
    char procname[MAX_FILENAME_BYTES+1];
    for(int i = 0; i < pid_count; i++)
    {
        if (show_but_pid){
            memset(procname, 0, sizeof(procname));
            int args_size = get_proc_data(pid_list[i], NULL, &state, &job_spec, procname, NULL);
            if (job_spec <= 0 || (req_js != -1 && req_js != job_spec))
                continue;
            char procargs[args_size];
            if (args_size > 0)
                get_proc_data(pid_list[i], NULL, NULL, NULL, NULL, procargs);
            if ((show_stopped && state == STOPPED) || (show_running && !(state == STOPPED || state == KILLED)))
                display_job(job_spec, pid_list[i], procname, procargs, args_size, show_pid, state != STOPPED);
        }
        else{
            get_proc_data(pid_list[i], NULL, &state, &job_spec, NULL, NULL);
            if (job_spec <= 0 || (req_js != -1 && req_js != job_spec))
                continue;
            if ((show_stopped && state == STOPPED) || (show_running && !(state == STOPPED || state == KILLED)))
                printf("%d\n", pid_list[i]);
        }
        if (req_js == job_spec){
            req_js = -1;
            break;
        }
    }
    if (req_js != -1){ /* Requested job spec was not found */
        printf("%s: %d: no such job\n", argv[0], req_js);
        return 1;
    }
    
    return 0;
}