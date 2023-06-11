#include "stdlib.h"

#define MAX_CMD_BUF_SIZE 256
#define ASCII_LOWERCASE_START 97
#define ASCII_LOWERCASE_END 122
#define ASCII_UPPERCASE_OFFSET 32
#define ASCII_BACKSPACE 127
#define ASCII_ESCAPE 27

static char to_upper(char ch)
{
    if (ch >= ASCII_LOWERCASE_START && ch <= ASCII_LOWERCASE_END)
        return (char)(ch - ASCII_UPPERCASE_OFFSET);
    return ch;
}

static void get_cmd_ext(char* cmd, char* ext)
{
    int sep_pos = strlen(cmd)-1;

    while (sep_pos > 0 && cmd[sep_pos] != '.')
    {
        sep_pos--;
    }
    
    if (sep_pos > 0)
        memcpy(ext, cmd+sep_pos+1, MAX_EXTNAME_BYTES);
}

static int read_cmd(char* buf, char* echo_buf)
{
    char shell_echo[4];
    int buf_size = 0;
    char cmd_ch;

    memset(shell_echo, 0, sizeof(shell_echo));

    while (1)
    {
        cmd_ch = getchar();

        /* Stop reading and exit when enter key (carriage return) is pressed by user */
        if (cmd_ch == '\r'){
            shell_echo[0] = '\r';
            shell_echo[1] = '\n';
            printf("%s", shell_echo);
            /* Null terminate the character echo buffer for upcoming character */
            shell_echo[1] = 0;
            break;
        }
        else if (cmd_ch == ASCII_BACKSPACE){
            if (buf_size == 0)
                continue;
            /* Use a combination of backspace and space characters to clear the last typed character on the screen */
            shell_echo[0] = '\b';
            shell_echo[1] = ' ';
            shell_echo[2] = '\b';
            printf("%s", shell_echo);
            shell_echo[1] = 0;
            /* Erase the last read character from the buffer */
            buf[buf_size-1] = 0;
            echo_buf[buf_size-1] = 0;
            buf_size--;
        }
        else{
            if (cmd_ch == ASCII_ESCAPE)
                continue;
            *shell_echo = cmd_ch;
            printf("%s", shell_echo);
            buf[buf_size] = to_upper(cmd_ch);
            echo_buf[buf_size] = cmd_ch;
            buf_size++;
        }
    }
}

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
            char cmd_ext[3];
            cmd_ext[0] = 0;
            get_cmd_ext(cmd_buf, cmd_ext);
            if (*cmd_ext == 0)
                memcpy(cmd_buf+strlen(cmd_buf), ".BIN", MAX_EXTNAME_BYTES+1);
            else if (memcmp(cmd_ext, "BIN", MAX_EXTNAME_BYTES) != 0){
                printf("%s: not an executable\n", cmd_buf);
                continue;
            }
            int fd = open_file(cmd_buf);
            if (fd < 0)
                printf("%s: command not found\n", echo_buf);
            else{
                close_file(fd);
                int cmd_pid = fork();
                if (cmd_pid == 0)
                    exec(cmd_buf);
                else
                    wait(cmd_pid);
            }
        }
    }
    
    return 0;
}