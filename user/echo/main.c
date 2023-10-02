/**
    Frostbyte kernel and operating system
    Copyright (C) 2023  Amol Dhamale <amoldhamale1105@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <sys/wait.h>
#include "flib.h"

int main(int argc, char** argv)
{
    if (argc > 1){
        switchpenv();
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
                            int status = get_pstatus();
                            printf("%d", WIFEXITED(status) ? WEXITSTATUS(status) : status);
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
                            memcpy((char*)env_var, &argv[i][j+1], env_len);
                            env_var[env_len] = 0;
                            printf("%s", getenv(env_var));
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