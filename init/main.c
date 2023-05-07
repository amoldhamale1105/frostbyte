/* The first user process (init.bin) with PID 1 */
#include "stdlib.h"

int main(void)
{
    printf("Init user process with PID %d started!\r\n", 1);
    return 0;
}