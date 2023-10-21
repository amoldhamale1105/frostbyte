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

#include "flib.h"

#define MAX_ENV_VAR_SIZE 64

static void print_usage(void)
{
    printf("Usage:");
    printf("\tunset [OPTION] name ...\n");
    printf("\tUnset values and attributes of shell variables.\n\n");
    printf("\tFor each NAME, remove the corresponding variable\n\n");
    printf("\t-h\tdisplay this help and exit\n");
}

void evaluate_env(char* in_env_str, int env_len, char* out_env_str)
{
    int in_pos = 0, out_pos = 0;
    int maxlen = MAX_ENV_VAR_SIZE;
    while (in_pos < env_len)
    {
        if (out_pos >= maxlen-1){
            out_pos = maxlen-1;
            break;
        }
        if (in_env_str[in_pos] == '$'){
            in_pos++;
            if (in_pos == env_len){
                out_env_str[out_pos++] = in_env_str[in_pos-1];
                break;
            }
            if (in_env_str[in_pos] == '$'){
                char* pid_str = itoa(getppid());
                int pid_len = strlen(pid_str);
                if (out_pos+pid_len >= maxlen-1)
                    pid_len = maxlen-2-out_pos;
                memcpy(out_env_str+out_pos, pid_str, pid_len);
                out_pos += pid_len;
                in_pos++;
            }
            else if (in_env_str[in_pos] == '?'){
                char* status_str = itoa(get_pstatus());
                int status_len = strlen(status_str);
                if (out_pos+status_len >= maxlen-1)
                    status_len = maxlen-2-out_pos;
                memcpy(out_env_str+out_pos, status_str, status_len);
                out_pos += status_len;
                in_pos++;
            }
            else{
                int nested_envlen = 0;
                while (in_pos < env_len && in_env_str[in_pos] != '$')
                {
                    nested_envlen++;
                    in_pos++;
                }
                char nested_env[nested_envlen+1];
                nested_env[nested_envlen] = 0;
                memcpy(nested_env, &in_env_str[in_pos-nested_envlen], nested_envlen);
                char* nested_envval = getenv(nested_env);
                if (nested_envval != NULL){
                    int vallen = strlen(nested_envval);
                    if (out_pos+vallen >= maxlen-1)
                        vallen = maxlen-2-out_pos;
                    memcpy(out_env_str+out_pos, nested_envval, vallen);
                    out_pos += vallen;
                }
            }
            
        }
        else{
            out_env_str[out_pos++] = in_env_str[in_pos++];
        }
    }
    out_env_str[out_pos] = 0;
}

int main(int argc, char** argv)
{
    if (argc < 2){
        printf("%s: bad usage\n", argv[0]);
        printf("Try \'%s -h\' for more information\n", argv[0]);
        return 1;
    }
    int ret = 0;
    if (argc > 1){
        int opt = 1;
        char env_name[MAX_ENV_VAR_SIZE];
        switchpenv();
        while (opt < argc)
        {
            if (argv[opt][0] != '-'){
                evaluate_env(argv[opt], strlen(argv[opt]), env_name);
                if (unsetenv(env_name)){
                    ret = 1;
                    printf("%s: \'%s\': failed to unset env variable\n", argv[0], env_name);
                }
                opt++;
                continue;
            }
            char* optstr = &argv[opt][1];
            while (*optstr)
            {
                switch (*optstr)
                {
                case 'h':
                    print_usage();
                    return 0;
                default:
                    printf("%s: invalid option \'%s\'\n", argv[0], argv[opt]);
                    printf("Try \'%s -h\' for more information\n", argv[0]);
                    return 1;
                }
                optstr++;
            }
            opt++;
        }
    }
    
    return ret;
}