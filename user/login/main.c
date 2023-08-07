#include "login.h"
#include "flib.h"
#include "signal.h"

int main(int argc, char** argv)
{
    char username[MAX_USERNAME_SIZE] = {0};
    char password[MAX_PASSWD_SIZE] = {0};
    const char* shell_args[] = { "-u", username, NULL };
    int attempts = MAX_LOGIN_ATTEMPTS;
    
    /* Ignore keyboard interrupts during login */
    signal(SIGINT, SIG_IGN);
    /* Access the passwd file from disk for login verification */
    int passwd_fd = open_file("PASSWD");
    if (passwd_fd < 0){
        printf("FATAL ERROR: Failed to open credentials file\n\n");
        kill(-1, SIGTERM);
        return 1;
    }
    int size = get_file_size(passwd_fd);
    char passwd_buf[size+1];
    int data_read = read_file(passwd_fd, passwd_buf, sizeof(passwd_buf));
    if (data_read != size){
        printf("FATAL ERROR: Failed to read credentials file\n\n");
        kill(-1, SIGTERM);
        return 1;
    }
    passwd_buf[size] = 0;
    close_file(passwd_fd);

    int namelen = 0, passlen = 0, nloff = 0;
    char* pbuf = passwd_buf;

    while (attempts)
    {
        printf("\n%s login: ", stringify_value(NAME));
        scanf("%s", username);
        if (*username == 0)
            continue;
        /* Search through the passwd file for entered username */
        while (nloff >= 0)
        {
            if (!pbuf || *pbuf == 0)
                break;
            while (*(pbuf+namelen) != ':')
            {
                namelen++;
            }
            /* Try matching entered password against registered user's password in file
               An entry of 'x' in the passwd file is interpreted as password-free login */
            if (memcmp(pbuf, username, namelen) == 0){
                username[namelen] = 0;
                pbuf += (namelen+1);
                while (*(pbuf+passlen) != ':')
                {
                    passlen++;
                }
                if (passlen == 1 && *pbuf == 'x')
                    exec("SH.BIN", shell_args);
                printf("Password: ");
                int passwd_size = read_passwd(password);
                if ((passwd_size != passlen) || memcmp(pbuf, password, passlen) != 0){
                    attempts--;
                    /* A small delay to generate an illusion of backend credential processing */
                    sleep(30);
                    printf("\n\nLogin incorrect. Try again\n");
                    namelen = passlen = 0;
                    break;
                }
                printf("\n\n");
                exec("SH.BIN", shell_args);
            }
            
            nloff = getline(pbuf);
            pbuf += (nloff+1);
            /* Skip newlines until valid data encountered */
            while (*pbuf == '\n')
            {
                nloff = getline(pbuf);
                pbuf += (nloff+1);
            }
            namelen = passlen = 0;
        }
        
        if (nloff < 0 || *pbuf == 0){
            attempts--;
            /* A small delay to generate an illusion of backend credential processing */
            sleep(30);
            printf("\nLogin incorrect. Try again\n");
            namelen = passlen = 0;
        }
        pbuf = passwd_buf;
        nloff = 0;
    }

    printf("Maximum incorrect login attempts exceeded\n\n");
    kill(-1, SIGTERM);
    
    return 1;
}