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
#include <stdbool.h>

static void print_usage(char ctrl)
{
    switch (ctrl)
    {
    case 'f':
        printf("Usage:");
        printf("\tfg [OPTION] [job_spec]\n");
        printf("\tMove job to the foreground.\n\n");
        printf("\tPlace the job identified by JOB_SPEC in the foreground, making it the\n\tcurrent job. If JOB_SPEC is not present, the shell's notion of the\n\tcurrent job is used\n\n");
        printf("\t-h\tdisplay this help and exit\n");
        break;
    case 'b':
        printf("Usage:");
        printf("\tbg [OPTION] [job_spec]\n");
        printf("\tMove job to the background.\n\n");
        printf("\tPlace the job identified by JOB_SPEC in the background, as if they\n\thad been started with `&'. If JOB_SPEC is not present, the shell's notion\n\tof the current job is used\n\n");
        printf("\t-h\tdisplay this help and exit\n");
        break;
    default:
        break;
    }
}

static int getdefjob(void)
{
    int def_js = -1;
    int pid_count = get_active_procs(NULL, false);
    int pid_list[pid_count];
    if (pid_count > 0){
        int js, state;
        get_active_procs(pid_list, false);
        for (int i = 0; i < pid_count; i++)
        {
            get_proc_data(pid_list[i], NULL, &state, &js, NULL, NULL);
            if (state != KILLED && js > 0){
                def_js = js;
                break;
            }
        }
    }

    return def_js;
}

int main(int argc, char** argv)
{
    int req_js = -1;
    char req_ctrl = 0;
    if (argc < 2)
        return 1;
    else{
        if (argv[1][0] == '-'){
            if (argc > 2){
                if (to_lower(argv[2][0]) == 'f' || to_lower(argv[2][0]) == 'b'){
                    switch (argv[1][1])
                    {
                    case 'h':
                        print_usage(to_lower(argv[2][0]));
                        return 0;
                    default:
                        printf("sh: %cg: invalid option \'%s\'\n", to_lower(argv[2][0]), argv[1]);
                        printf("Try \'%cg -h\' for more information\n", to_lower(argv[2][0]));
                        break;
                    }
                }
            }
            return 1;
        }
        else{
            if (to_lower(argv[2][0]) == 'f' || to_lower(argv[2][0]) == 'b'){
                req_js = getdefjob();
                if (req_js < 0){
                    printf("sh: %cg: current: no such job\n", to_lower(argv[1][0]));
                    return 1;
                }
            }
            else
                req_js = atoi(argv[1]);
            if (req_js > 0){
                if (argc > 2){
                    if (to_lower(argv[2][0]) == 'f' || to_lower(argv[2][0]) == 'b')
                        req_ctrl = to_lower(argv[2][0]);
                    else
                        return 1;
                }
                else
                    return 1;
            }
            else{
                if (argc > 2){
                    if (to_lower(argv[2][0]) == 'f' || to_lower(argv[2][0]) == 'b'){
                        printf("sh: %cg: bad usage\n", to_lower(argv[2][0]));
                        printf("Try \'%cg -h\' for more information\n", to_lower(argv[2][0]));
                    }
                }
                return 1;
            }
        }
    }
    
    int jpid = getjpid(req_js);
    if (jpid > 0){
        int state;
        get_proc_data(jpid, NULL, &state, NULL, NULL, NULL);
        if (!(state == STOPPED || state == KILLED) && req_ctrl == 'b'){
            printf("sh: bg: job %d is already in background\n", req_js);
            return 1;
        }
    }
    else{
        printf("sh: %cg: %d: no such job\n", req_ctrl, req_js);
        return 1;
    }
    if (setjobctl(req_js, req_ctrl == 'f' ? 0 : 1) < 0){
        printf("sh: %cg: %d: failed to perform control operation\n", req_ctrl, req_js);
        return 1;
    }
    if (kill(jpid, SIGCONT) < 0){
        printf("sh: %cg: %d: failed to resume job\n", req_ctrl, req_js);
        return 1;
    }
    
    char procname[MAX_FILENAME_BYTES+1];
    int args_size = get_proc_data(jpid, NULL, NULL, NULL, procname, NULL);
    int args_pos = 0;
    char procargs[args_size];
    if (args_size > 0)
        get_proc_data(jpid, NULL, NULL, NULL, NULL, procargs);

    switch (req_ctrl)
    {
    case 'f':
        printf("%s ", procname);
        while (args_pos < args_size)
        {
            printf("%s ", procargs+args_pos);
            args_pos += (strlen(procargs+args_pos)+1);
        }
        printf("\n");
        break;
    case 'b':
        printf("[%d]  %s ", req_js, procname);
        while (args_pos < args_size)
        {
            printf("%s ", procargs+args_pos);
            args_pos += (strlen(procargs+args_pos)+1);
        }
        printf("%c\n", '&');
        break;
    default:
        break;
    }

    return 0;
}