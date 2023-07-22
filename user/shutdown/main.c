#include "flib.h"
#include "signal.h"

static void print_usage(void)
{
    printf("Usage:\n");
    printf("\tshutdown [OPTION]\n");
    printf("Shut down the system (Stop all active processes and disable interrupt handling. No ACPI mapping)\n\n");
    printf("\t-h\tdisplay this help and exit\n");
}

int main(int argc, char** argv)
{
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
    }
    /* A negative PID will send signals to all processes in the system
       Negative PID with SIGTERM implies a shutdown request */
    if (kill(-1, SIGTERM) < 0){
        printf("%s: failed to shut down the system\n", argv[0]);
        return 1;
    }

    return 0;
}