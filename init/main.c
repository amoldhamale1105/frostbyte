/* The first user process (init.bin) with PID 1 */
#include "stdlib.h"

int main(void)
{
    uint64_t sched_counter = 0;

    while (1)
    {
        printf("Init user process with PID %d running %u\r\n", 1, sched_counter++);
        /* This should sleep for 1 second given that 1 tick = 10 ms */
        sleepu(100);
    }

    return 0;
}