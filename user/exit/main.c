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