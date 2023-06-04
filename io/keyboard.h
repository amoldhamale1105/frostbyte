#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* A circular buffer for key strokes */
struct KeyBuffer
{
    char buf[500];
    int front;  /* buffer read position */
    int end;    /* buffer write position */
};

#define MAX_KEY_BUF_SIZE 500

char read_key_buffer(void);
void capture_key(void);

#endif