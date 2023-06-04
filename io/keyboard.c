#include "keyboard.h"
#include "uart.h"
#include <process/process.h>

static struct KeyBuffer key_buf = {
    .buf = {0},
    .front = 0,
    .end = 0,
};

static void write_key_buffer(char ch)
{
    int next_write_pos = (key_buf.end + 1) % MAX_KEY_BUF_SIZE;

    /* Check if the buffer is full */
    if (next_write_pos == key_buf.front)
        return;

    key_buf.buf[key_buf.end] = ch;
    key_buf.end = next_write_pos;
}

char read_key_buffer(void)
{
    /* If buffer is empty, sleep on keyboard input event */
    if (key_buf.front == key_buf.end)
        sleep(KEYBOARD_INPUT);

    char ch = key_buf.buf[key_buf.front];
    key_buf.front = (key_buf.front + 1) % MAX_KEY_BUF_SIZE;
    
    return ch;
}

void capture_key(void)
{
    /* Capture the pressed key from uart module and push it to the circular buffer */
    write_key_buffer(read_char());
    /* Wake up all processes sleeping on keyboard input event */
    wake_up(KEYBOARD_INPUT);
}
