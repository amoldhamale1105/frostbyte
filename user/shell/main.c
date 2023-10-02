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

#include "shell.h"
#include "signal.h"
#include <sys/wait.h>

void sighandler(int signum)
{
    if (signum == SIGINT){
        printf("^C\n");
        interrupted = true;
        /* Re-register handler since kernel resets it to default after first invokation */
        signal(SIGINT, sighandler);
    }
}

int main(int argc, char** argv)
{
    char cmd_buf[MAX_CMD_BUF_SIZE];
    char echo_buf[MAX_CMD_BUF_SIZE];
    int cmd_size = 0;
    int wstatus;

    char prompt_suffix = '$';
    char* username = getenv("USER");
    if (username && strlen(username) == 4 && memcmp(username, "root", 4) == 0)
        prompt_suffix = '#';

    /* Register custom handler for keyboard interrupt so that the shell does not get terminated on Ctrl+C from user */
    signal(SIGINT, sighandler);
    signal(SIGTSTP, SIG_IGN);

    while (1)
    {
        printf("%s@%s:~%c ", username, stringify_value(NAME), prompt_suffix);
        memset(cmd_buf, 0, sizeof(cmd_buf));
        memset(echo_buf, 0, sizeof(echo_buf));
        cmd_size = read_cmd(cmd_buf, echo_buf);
        if (interrupted){
            interrupted = false;
            continue;
        }
        
        if (cmd_size > 0){
            int cmd_pos, arg_count;
            char* cmd_ext;
            char* args[MAX_PROG_ARGS];
            arg_count = get_cmd_info(cmd_buf, echo_buf, &cmd_pos, &cmd_ext, args);
            if (cmd_ext == NULL){
                char* cmd_end = cmd_buf+cmd_pos+strlen(cmd_buf+cmd_pos);
                memcpy(cmd_end, ".BIN", MAX_EXTNAME_BYTES+1);
                *(cmd_end+MAX_EXTNAME_BYTES+1) = 0;
            }
            else if (memcmp(cmd_ext, "BIN", MAX_EXTNAME_BYTES) != 0){
                printf("%s: not an executable\n", echo_buf+cmd_pos);
                continue;
            }
            /* Forbid direct execution of init and login programs by the user */
            if (memcmp(cmd_buf+cmd_pos, "INIT.BIN", strlen(cmd_buf+cmd_pos)) == 0 ||
                memcmp(cmd_buf+cmd_pos, "LOGIN.BIN", strlen(cmd_buf+cmd_pos)) == 0){
                printf("%s: %s - Operation not permitted\n", argv[0], cmd_buf+cmd_pos);
                continue;
            }
            if (memcmp(cmd_buf+cmd_pos, "FG.BIN", 6) == 0 || memcmp(cmd_buf+cmd_pos, "BG.BIN", 6) == 0){
                char jctl_cmd[] = "JOBCTL.BIN";
                int jctl_len = strlen(jctl_cmd);
                memcpy(cmd_buf+cmd_pos, jctl_cmd, jctl_len);
                cmd_buf[cmd_pos+jctl_len] = 0;
                if (!args[0])
                    args[0] = echo_buf+cmd_pos;
                args[1] = echo_buf+cmd_pos;
            }
            int fd = open_file(cmd_buf+cmd_pos);
            if (fd < 0)
                printf("%s: command not found\n", echo_buf+cmd_pos);
            else{
                close_file(fd);
                int cmd_pid = fork();
                if (cmd_pid == 0)
                    exec(cmd_buf+cmd_pos, (const char**)args);
                else{
                    /* Don't make the parent wait since it's a background process, so that the shell becomes available to subsequent commands */
                    if (arg_count > 0 && strlen(args[arg_count-1]) == 1 && args[arg_count-1][0] == '&'){
                        int jobspec;
                        get_proc_data(cmd_pid, NULL, NULL, &jobspec, NULL, NULL);
                        printf("[%d] %d\n", jobspec, cmd_pid);
                        continue;
                    }
                    int wpid = waitpid(cmd_pid, &wstatus, WUNTRACED);
                    if (WIFSIGNALED(wstatus)){
                        switch (WTERMSIG(wstatus))
                        {
                        case SIGINT:
                            printf("\n");
                            break;
                        case SIGABRT:
                            printf("Aborted\n");
                            break;
                        case SIGKILL:
                            printf("Killed\n");
                            break;
                        case SIGTERM:
                            printf("Terminated\n");
                        default:
                            break;
                        }
                    }
                    else if (WIFSTOPPED(wstatus)){
                        int job_spec;
                        char procname[MAX_FILENAME_BYTES+1];
                        get_proc_data(wpid, NULL, NULL, &job_spec, procname, NULL);
                        printf("^Z\n[%d]  Stopped\t%s\n", job_spec, procname);
                    }
                }
            }
        }
    }
    
    return 0;
}