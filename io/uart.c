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

#include "uart.h"
#include "keyboard.h"
#include <lib/lib.h>

unsigned char read_char(void)
{
    /* Read the data buffer register */
    return in_word(UART0_DR);
}

void write_char(unsigned char c)
{
    /* Spin until bit 3 of the flags register is 0 meaning that the device is no more busy */
    while (in_word(UART0_FR) & (1 << 3));
    out_word(UART0_DR, c);
}

void write_string(const char *str)
{
    while (*str)
    {
        write_char(*str++);
    }   
}

void uart_handler(void)
{
    /* Check if it is a receiving UART interrupt by reading bit 4 of UART masked interrupt status register */
    uint32_t status = in_word(UART0_MIS);

    if (status & (1 << 4)){
        /* Read all characters in the FIFO buffer until the RXFE (Receive FIFO empty) bit of flags register is set */
        while (!(in_word(UART0_FR) & (1 << 4)))
        {
            capture_key();
        }
        /* Clear the interrupt by setting bit 4 of the interrupt clear register */
        out_word(UART0_ICR, (1 << 4));
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
    /* Write 1 to bit 4 of interrupt mask set or clear register to enable interrupts */
    out_word(UART0_IMSC, (1 << 4));
    /* Set bits 0,8,9 of control register to high to enable uart0, transmit and receive */
    out_word(UART0_CR, (1 << 8) | (1 << 9) | 1);
}