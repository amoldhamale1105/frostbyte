/* The first user process (init.bin) with PID 1 */
#include "stdlib.h"

int main(void)
{
    uint64_t sched_counter = 0;
    const char* file1 = "TEST.BIN";
    const char* file2 = "DYNCAST.CPP";
    const char* file3 = "MANIFEST.XML";

    int file1_fd = open_file((char*)file1);
    if (file1_fd == -1)
        printf("Failed to open file %s\n", file1);
    else
        printf("%s file descriptor is %d\n", file1, file1_fd);
    
    int file2_fd = open_file((char*)file2);
    if (file2_fd == -1)
        printf("Failed to open file %s\n", file2);
    else
        printf("%s file descriptor is %d\n", file2, file2_fd);

    int file3_fd = open_file((char*)file3);
    if (file3_fd == -1)
        printf("Failed to open file %s\n", file3);
    else
        printf("%s file descriptor is %d\n", file3, file3_fd);

    while (sched_counter < 5)
    {
        printf("Init user process with PID %d running %u\r\n", 1, sched_counter++);
        /* This should sleep for 1 second given that 1 tick = 10 ms */
        sleepu(100);
    }

    close_file(file1_fd);
    close_file(file2_fd);
    close_file(file3_fd);

    return 0;
}