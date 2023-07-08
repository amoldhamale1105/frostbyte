#include "stdlib.h"

int main(int argc, char** argv)
{
    if (argc > 1){
        for(int i = 0; i < argc; i++)
        {
            printf("arg%d: %s\n", i, argv[i]);
        }
        /* An exit test code passed as first argument */
        if (memcmp(argv[1], "ret", 3) == 0)
            return 0;
    }
    while (1)
    {
        sleep(10);
    }
    
    return 0;
}