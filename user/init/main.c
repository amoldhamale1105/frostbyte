/* The first user process (init.bin) with PID 1 */
#include "flib.h"
#include <stddef.h>

int main(void)
{
    printf("\nWelcome to %s (A minimalistic aarch64 kernel)\n\n", stringify_value(NAME));
    int pid = fork();
    
    if (pid == 0) /* Child process */
        exec("LOGIN.BIN", NULL);
    else if (pid == -1){
        printf("Init process failed to fork!\n");
        return 1;
    }
    
    /* Wait for death of own children and processes orphaned by exiting parents */
    while ((pid = wait(-1)) != -1)
    {
        /* Consider respawning a dead child here otherwise continue */
    }    

    return 0;
}