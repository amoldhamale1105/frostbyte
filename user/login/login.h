#ifndef LOGIN_H
#define LOGIN_H

#include <stddef.h>

#define MAX_LOGIN_ATTEMPTS 5
#define MAX_USERNAME_SIZE 50
#define MAX_PASSWD_SIZE 50

int getline(char* file_buf);
int read_passwd(char* buf);

#endif