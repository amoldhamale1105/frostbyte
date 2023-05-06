#include "stdint.h"
#include "print.h"
#include "libc.h"
#include "irq.h"
#include "uart.h"
#include "handler.h"

void enable_timer(void);
uint32_t read_timer_status(void);
void set_timer_interval(uint32_t value);
uint32_t read_timer_freq(void);

static uint32_t timer_interval = 0;
static uint64_t ticks = 0;

void init_interrupt_controller(void)
{
    /* Disable all interrupts initially to enable selectively later on */
    out_word(DISABLE_BASIC_IRQS, 0xffffffff);
    out_word(DISABLE_IRQS_1, 0xffffffff);
    out_word(DISABLE_IRQS_2, 0xffffffff);

    /* Set bit 25 in intr enable register 2 to enable IRQ 57 of UART */
    out_word(ENABLE_IRQS_2, (1 << 25));
}

void init_timer(void)
{
    /* Save the timer interval for the handler to retrigger when the interrupt is triggered for the first time */
    timer_interval = read_timer_freq() / 100;
    enable_timer();
    /* Set bit 1 (IRQ enable) of the core timer status register to enable timer interrupts */
    out_word(CNTP_EL0, (1 << 1));
}

static void timer_interrupt_handler(void)
{
    uint32_t status = read_timer_status();
    /* If bit 2 is set, it means the timer has fired */
    if (status & (1 << 2)){
        ticks++;
        /* Print every second (timer fires every 10 ms) */
        if (ticks % 100 == 0)
            printk("Timer fired, tick count: %d\r\n", ticks);
        set_timer_interval(timer_interval);
    }
}

static uint32_t get_irq_number(void)
{
    /* Read the basic pending register to get the asserted IRQ number */
    return in_word(IRQ_BASIC_PENDING);
}

void handler(struct ContextFrame* ctx)
{
    uint32_t irq;
    switch (ctx->trapno)
    {
    case 1:
        printk("Sync exception at %x: %x\r\n", ctx->elr, ctx->esr);
        while(1);
        break;
    /* Hardware interrupt */
    case 2:
        /* Read the interrupt source register to check what kind of hardware interrupt it is */
        irq = in_word(CNTP_STATUS_EL0);
        /* High bit 1 indicates timer interrupt */
        if (irq & (1 << 1))
            timer_interrupt_handler();
        else{
            irq = get_irq_number();
            /* Bit 19 of the interrupt pending register is for IRQ 57 i.e. UART interrupt */
            if (irq & (1 << 19))
                uart_handler();
            else{
                printk("Unknown hardware interrupt\r\n");
                while(1);
            }
        }
        break;
    
    default:
        printk("Unknown exception\r\n");
        while(1);
        break;
    }
}