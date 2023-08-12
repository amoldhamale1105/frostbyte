#ifndef UART_H
#define UART_H

#include <memory/memory.h>

#define IO_BASE_ADDR    TO_VIRT(0x3f200000)

#define UART0_DR        IO_BASE_ADDR + 0x1000 /* Data register */
#define UART0_FR        IO_BASE_ADDR + 0x1018 /* Flags register */
#define UART0_CR        IO_BASE_ADDR + 0x1030 /* Control register */
#define UART0_LCRH      IO_BASE_ADDR + 0x102c /* Line control register */
#define UART0_FBRD      IO_BASE_ADDR + 0x1028 /* Fractional part of baud rate divisor register */
#define UART0_IBRD      IO_BASE_ADDR + 0x1024 /* Integral part of baud rate divisor register */
#define UART0_IMSC      IO_BASE_ADDR + 0x1038 /* Interrupt mask set/clear register */
#define UART0_RIS       IO_BASE_ADDR + 0x103c /* Raw interrupt status register */
#define UART0_MIS       IO_BASE_ADDR + 0x1040 /* Masked interrupt status register */
#define UART0_ICR       IO_BASE_ADDR + 0x1044 /* Interrupt clear register */

unsigned char read_char(void);
void write_char(unsigned char c);
void write_string(const char *str);
void init_uart(void);
void uart_handler(void);

#endif
