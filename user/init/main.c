/* The first user process (init.bin) with PID 1 */
#include "stdlib.h"
#include "stddef.h"

int main(void)
{
    int pid = fork();
    const char* args[] = {"args", "passed", "to", "test", "program", NULL};
    
    if (pid == 0) /* Child process */
        exec("TEST.BIN", args);
    else if (pid == -1)
        printf("Init process failed to fork!\n");
    else{ /* Parent process */
        /* Wait for the child to finish and then clean up its resources once it's done */
        wait(pid);
    }

    return 0;
}