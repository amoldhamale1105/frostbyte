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
#include <stdbool.h>

#define MAX_ENV_VAR_SIZE 64
#define MAX_ENV_VAL_SIZE 128

enum En_EnvType
{
    NAME = 0,
    VALUE
};

static void print_usage(void)
{
    printf("Usage:");
    printf("\texport [OPTION] name[=value] ...\n");
    printf("\tSet export attribute for shell variables.\n\n");
    printf("\tMarks each NAME for automatic export to the environment of subsequently\n\texecuted commands. If VALUE is supplied, assign VALUE before exporting.\n\n");
    printf("\t-h\tdisplay this help and exit\n");
}

bool is_env_valid(char ch, int pos)
{
    return (ch >= BASE_NUMERIC_ASCII && ch <= BASE_NUMERIC_ASCII+9 && pos > 0) ||
            (ch >= ASCII_LOWERCASE_START && ch <= ASCII_LOWERCASE_END) ||
            (ch >= ASCII_LOWERCASE_START-ASCII_UPPERCASE_OFFSET && ch <= ASCII_LOWERCASE_END-ASCII_UPPERCASE_OFFSET) ||
            (ch == '_');
}

void evaluate_env(int type, char* in_env_str, int env_len, char* out_env_str)
{
    int in_pos = 0, out_pos = 0;
    int maxlen = type == NAME ? MAX_ENV_VAR_SIZE : MAX_ENV_VAL_SIZE;
    while (in_pos < env_len)
    {
        if (out_pos >= maxlen-1){
            out_pos = maxlen-1;
            break;
        }
        if (in_env_str[in_pos] == '$'){
            in_pos++;
            if (in_pos == env_len){
                if (type == NAME)
                    out_pos = 0;
                else
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
                bool env_valid = true;
                while (in_pos < env_len && in_env_str[in_pos] != '$')
                {
                    if (!(env_valid = is_env_valid(in_env_str[in_pos], nested_envlen)))
                        break;
                    nested_envlen++;
                    in_pos++;
                }
                if (!env_valid){
                    out_pos = 0;
                    break;
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
            if (type == NAME && !is_env_valid(in_env_str[in_pos], in_pos)){
                out_pos = 0;
                break;
            }
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
        char env_val[MAX_ENV_VAL_SIZE];
        switchpenv();
        while (opt < argc)
        {
            if (argv[opt][0] != '-'){
                int eq_index = find('=', argv[opt]);
                if (eq_index == 0){
                    ret = 1;
                    printf("%s: \'%s\': not a valid identifier\n", argv[0], argv[opt]);
                }
                else if (eq_index > 0){
                    evaluate_env(NAME, argv[opt], eq_index, env_name);
                    if (*env_name == 0){
                        ret = 1;
                        printf("%s: \'%s\': not a valid identifier\n", argv[0], argv[opt]);
                    }
                    else{
                        evaluate_env(VALUE, &argv[opt][eq_index+1], strlen(&argv[opt][eq_index+1]), env_val);
                        if (setenv(env_name, env_val, 1)){
                            ret = 1;
                            printf("%s: \'%s\': failed to set env variable\n", argv[0], env_name);
                        }
                    }
                }
                else{
                    evaluate_env(NAME, argv[opt], strlen(argv[opt]), env_name);
                    if (*env_name == 0){
                        ret = 1;
                        printf("%s: \'%s\': not a valid identifier\n", argv[0], argv[opt]);
                    }
                    else{
                        if (setenv(env_name, "", 0)){
                            ret = 1;
                            printf("%s: \'%s\': failed to set env variable\n", argv[0], env_name);
                        }
                    }
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