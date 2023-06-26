#include "stdlib.h"

int main(int argc, char** argv)
{
    if (argc > 1){
        if (memcmp(argv[1], "arg", 3) == 0){
            for(int i = 0; i < argc; i++)
            {
                printf("arg%d: %s\n", i, argv[i]);
            }
            return 0;
        }
    }
    while (1)
    {
        sleep(10);
    }
    
    return 0;
}