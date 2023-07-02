#include "stdlib.h"
#include "signal.h"

int main(int argc, char** argv)
{
    /* A negative PID will send signals to all processes in the system
       Negative PID with SIGTERM implies a shutdown request */
    if (kill(-1, SIGTERM) < 0){
        printf("%s: failed to shut down the system\n", argv[0]);
        return 1;
    }

    return 0;
}