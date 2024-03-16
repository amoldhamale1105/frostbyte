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

/* The first user process (init.bin) with PID 1 */
#include "flib.h"
#include "signal.h"
#include <stddef.h>
#include <stdbool.h>

int login_shell_pid;

static int respawn(char* procname, const char* args[], int* new_pid)
{
    int pid, ret = 0;
    
    pid = fork();
    if (pid == 0)
        ret = exec(procname, args);
    else if (pid == -1){
        printf("Init process failed to respawn %s\n", procname);
        ret = 1;
    }
    if (new_pid)
        *new_pid = pid;
    
    return ret;
}

int main(void)
{
    printf("\nWelcome to %s (A minimalistic %s kernel)\n", stringify_value(NAME), stringify_value(ARCH));
    int pid = fork();
    
    if (pid == 0) /* Child process */
        exec("LOGIN.BIN", NULL);
    else if (pid == -1){
        printf("Init process failed to spawn login shell!\n");
        return 1;
    }
    /* Save the main shell PID to be respawned on exit */
    login_shell_pid = pid;
    
    /* Wait for death of own children and processes orphaned by exiting parents */
    while ((pid = wait((void*) 0)) != -1)
    {
        if (pid == login_shell_pid){
            /* Hang up all processes since the user has logged out */
            kill(-1, SIGHUP);
            msleep(50);
            if (respawn("LOGIN.BIN", NULL, &login_shell_pid) != 0){
                kill(-1, SIGTERM);
                return 1;
            }
        }
    }

    return 0;
}