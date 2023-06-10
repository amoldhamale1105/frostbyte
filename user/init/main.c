/* The first user process (init.bin) with PID 1 */
#include "stdlib.h"

int main(void)
{
    int pid = fork();
    
    if (pid == 0) /* Child process */
        exec("SHELL.BIN");
    else if (pid == -1)
        printf("Init process failed to fork!\n");
    else{ /* Parent process */
        /* Wait for the child to finish and then clean up its resources once it's done */
        waitu(pid);
    }

    return 0;
}