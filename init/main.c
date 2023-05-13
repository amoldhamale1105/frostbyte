/* The first user process (init.bin) with PID 1 */
#include "stdlib.h"

int main(void)
{
    uint64_t sched_counter = 0;

    while (1)
    {
        if (sched_counter % 100000000 == 0)
            printf("Init user process with PID %d running %u\r\n", 1, sched_counter);
        sched_counter++;
    }

    return 0;
}