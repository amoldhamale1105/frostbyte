#include "stdlib.h"

int main(int argc, char** argv)
{
    printf("Test user process (PID %d) started\n", getpid());
    for(int i = 0; i < argc; i++)
    {
        printf("arg%d: %s\n", i, argv[i]);
    }
    
    return 0;
}