#include "stdlib.h"
#include "signal.h"
#include <stddef.h>

int main(int argc, char** argv)
{
    int pid, signal = SIGTERM;
    if (argc < 2){
        printf("%s: Needs PID as argument\n", argv[0]);
        return 1;
    }
    if (argc >= 2){
        if (argc > 2){
            signal = abs(atoi(argv[1]));
            pid = atoi(argv[2]);
        }
        else
            pid = atoi(argv[1]);
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
        printf("%s: Failed to send signal %d to PID %d\n", signal, pid);
    
    return 0;
}