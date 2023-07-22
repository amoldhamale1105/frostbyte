#include "flib.h"
#include "signal.h"
#include <stddef.h>

static void print_usage(void)
{
    printf("Usage:\n");
    printf("\tkill [OPTION] <pid>\n");
    printf("Send a signal to a process\n\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf("\t-sig\tsignal number (Refer to POSIX signals)\n");
}

int main(int argc, char** argv)
{
    int pid, signal = SIGTERM;
    if (argc < 2){
        printf("%s: bad usage\n", argv[0]);
        printf("Try \'%s -h\' for more information\n", argv[0]);
        return 1;
    }
    if (argc > 1){
        int arg_len = strlen(argv[1]);
        if (argc > 2){
            if (argv[1][0] != '-' || arg_len > 2){
                printf("%s: invalid option \'%s\'\n", argv[0], argv[1]);
                printf("Try \'%s -h\' for more information\n", argv[0]);
                return 1;
            }
            /* Used to create the delusion of an option with argument */
            signal = abs(atoi(argv[1]));
            pid = atoi(argv[2]);
        }
        else{
            if (argv[1][0] == '-'){
                if (arg_len == 2){
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
            pid = atoi(argv[1]);
        }
    }
    
    int state = -1;
    get_proc_data(pid, NULL, &state, NULL);
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