#include "uart.h"
#include "libc.h"

unsigned char read_char(void)
{
    /* Wait until the there is some data in the receive buffer i.e. bit 4 of flag register becomes high */
    while (in_word(UART0_FR) & (1 << 4));
    /* Bit 4 clear implies that the data is ready to be read */
    return in_word(UART0_DR);
}

void write_char(unsigned char c)
{
    /* Spin until bit 5 of the flags register is 0 meaning that transmit FIFO is no more full */
    while (in_word(UART0_FR) & (1 << 5));
    out_word(UART0_DR, c);
}

void write_string(const char *str)
{
    while (*str)
    {
        write_char(*str++);
    }   
}

void init_uart(void)
{
    out_word(UART0_CR, 0);
    /* Based on BCM2835 ARM peripherals document, baud divisor = UART clk / 16 * baud rate)
       UART clock = 48 MHz, baud rate = 115200 */
    out_word(UART0_IBRD, 26);
    out_word(UART0_FBRD, 0);
    /* Write 1 to bit 4 of line control register to enable FIFO buffer
       Write 1 to bit 5 and 6 to enable 8-bit data mode */
    out_word(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
    /* Write 0 to interrupt mask set or clear register to mask all interrupts */
    out_word(UART0_IMSC, 0);
    /* Set bits 0,8,9 of control register to high to enable uart0, transmit and receive */
    out_word(UART0_CR, (1 << 8) | (1 << 9) | 1);
}