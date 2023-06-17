#include "shell.h"

int main(void)
{
    char cmd_buf[MAX_CMD_BUF_SIZE];
    char echo_buf[MAX_CMD_BUF_SIZE];
    int cmd_size = 0;

    /* Run the shell indefinitely while the system is up */
    while (1)
    {
        printf("root@pious:~# ");
        memset(cmd_buf, 0, sizeof(cmd_buf));
        memset(echo_buf, 0, sizeof(echo_buf));
        cmd_size = read_cmd(cmd_buf, echo_buf);
        
        if (cmd_size > 0){
            int cmd_pos;
            char* cmd_ext;
            char* args[MAX_PROG_ARGS];
            get_cmd_info(cmd_buf, echo_buf, &cmd_pos, &cmd_ext, args);
            if (cmd_ext == NULL){
                char* cmd_end = cmd_buf+cmd_pos+strlen(cmd_buf+cmd_pos);
                memcpy(cmd_end, ".BIN", MAX_EXTNAME_BYTES+1);
                *(cmd_end+MAX_EXTNAME_BYTES+1) = 0;
            }
            else if (memcmp(cmd_ext, "BIN", MAX_EXTNAME_BYTES) != 0){
                printf("%s: not an executable\n", echo_buf+cmd_pos);
                continue;
            }
            int fd = open_file(cmd_buf+cmd_pos);
            if (fd < 0)
                printf("%s: command not found\n", echo_buf+cmd_pos);
            else{
                close_file(fd);
                int cmd_pid = fork();
                if (cmd_pid == 0)
                    exec(cmd_buf+cmd_pos, (const char**)args);
                else
                    wait(cmd_pid);
            }
        }
    }
    
    return 0;
}