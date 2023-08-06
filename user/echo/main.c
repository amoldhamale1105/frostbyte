#include <sys/wait.h>
#include "flib.h"

int main(int argc, char** argv)
{
    if (argc > 1){
        /* Start from the second argument since the first one is program name */
        for(int i = 1; i < argc; i++)
        {
            char* arg = argv[i];
            int arg_len = strlen(argv[i]);
            for(int j = 0; j < arg_len; j++)
            {
                if (argv[i][j] == '$'){
                    if (j < arg_len-1){
                        if (argv[i][j+1] == '$'){
                            printf("%d", getppid());
                            j++;
                            continue;
                        }
                        else if (argv[i][j+1] == '?'){
                            printf("%d", WEXITSTATUS(get_pstatus()));
                            j++;
                            continue;
                        }
                        else{
                            int env_len = 0;
                            while ((j+1+env_len) < arg_len)
                            {
                                if (argv[i][j+1+env_len] == '$')
                                    break;
                                env_len++;
                            }
                            char env_var[env_len+1];
                            memcpy(env_var, (char*)argv+i+j+1, env_len);
                            env_var[env_len] = 0;
                            /* Evaluate and print environment variable here */
                            j += env_len;
                            continue;
                        }
                    }
                }
                printf("%c", argv[i][j]);
            }
            printf(" ");
        }
    }
    printf("\n");
    
    return 0;
}