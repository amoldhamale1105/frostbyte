#include "shell.h"
#include "stdbool.h"

bool interrupted = false;

int get_cmd_info(char* cmd, char* echo, int* cmd_pos, char** ext, char** echo_args)
{
    char* cmd_ptr = NULL;
    char* echo_ptr = NULL;
    char* arg_ptr = NULL;
    bool new_arg = false;
    int arg_count = 0;

    if (cmd == NULL || echo == NULL || cmd_pos == NULL || ext == NULL || echo_args == NULL)
        return -1;
    
    *cmd_pos = 0;
    *ext = NULL;
    /* Check if there are any leading spaces to adjust command start position */
    while (cmd[*cmd_pos] == ' ')
    {
        (*cmd_pos)++;
    }
    cmd_ptr = cmd + *cmd_pos;
    arg_ptr = cmd_ptr;
    /* Traverse until first space which marks end of command and beginning of args if any */
    while (*arg_ptr != ' ' && *arg_ptr != '\0')
    {
        arg_ptr++;
    }
    *arg_ptr = 0;
    *ext = arg_ptr;
    /* NOTE Reverse searching is needed because filename can contain dots but the last dot will always signify extension separator */
    while (*ext != cmd_ptr && *(*ext-1) != '.')
    {
        (*ext)--;
    }
    if (*ext == cmd_ptr)
        *ext = NULL;
    /* Null terminate args in the echo buffer and append their addresses to the argument list */
    echo_ptr = echo + *cmd_pos + buf_offset(cmd_ptr, arg_ptr);
    *echo_ptr++ = 0;
    while (*echo_ptr != '\0')
    {
        if (*echo_ptr == ' '){
            if (new_arg){
                *echo_ptr = 0;
                new_arg = false;
            }
        }
        else{
            if (!new_arg){
                new_arg = true;
                echo_args[arg_count++] = echo_ptr;
            }
        }
        echo_ptr++;
    }
    /* Null terminate the array of arguments */
    echo_args[arg_count] = NULL;
    return arg_count;
}

int read_cmd(char* buf, char* echo_buf)
{
    char shell_echo[4];
    int buf_size = 0;
    char cmd_ch;

    memset(shell_echo, 0, sizeof(shell_echo));

    while (1)
    {
        cmd_ch = getchar();
        if (interrupted)
            break;

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
            if (cmd_ch == ASCII_CTRL_C || cmd_ch == ASCII_ESCAPE)
                continue;
            *shell_echo = cmd_ch;
            printf("%s", shell_echo);
            buf[buf_size] = to_upper(cmd_ch);
            echo_buf[buf_size] = cmd_ch;
            buf_size++;
        }
    }

    return buf_size;
}