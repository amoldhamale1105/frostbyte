#include "stdlib.h"

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
    printf("Usage:\n");
    printf("\tps [OPTION] [pid_count]\n");
    printf("Report a snapshot of current processes\n\n");
    printf("\t-h\tdisplay this help and exit\n");
}

int main(int argc, char** argv)
{
    int display_num = DEF_DISPLAY_PROCS;
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
        int num_param = atoi(argv[1]);
        if (num_param <= 0){
            printf("%s: %s Invalid argument\n", argv[0], argv[1]);
            return 1;
        }
        display_num = num_param;
    }

    int pid_list[display_num];
    int pid_count = get_active_procs(pid_list);
    display_num = pid_count < display_num ? pid_count : display_num;

    int ppid, state;
    char procname[MAX_FILENAME_BYTES+1];

    printf("PID    PPID    STATE    PROC NAME\n");
    printf("---------------------------------\n");
    for(int i = 0; i < display_num; i++)
    {
        memset(procname, 0, sizeof(procname));
        get_proc_data(pid_list[i], &ppid, &state, procname);
        printf("%d\t%d\t%c        %s\n", pid_list[i], ppid, state_rep(state), procname);
    }
    
    return 0;
}