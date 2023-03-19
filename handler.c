#include "stdint.h"
#include "print.h"
#include "libc.h"
#include "irq.h"

void enable_timer(void);
uint32_t read_timer_status(void);
void set_timer_interval(uint32_t value);
uint32_t read_timer_freq(void);

static uint32_t timer_interval = 0;
static uint64_t ticks = 0;

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
            printk("Timer fired, tick count: %d \r\n", ticks);
        set_timer_interval(timer_interval);
    }
}

void handler(uint64_t id, uint64_t esr, uint64_t elr)
{
    uint32_t irq;
    switch (id)
    {
    case 1:
        printk("Sync exception at %x: %x\r\n", elr, esr);
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
            printk("Unknown hardware interrupt\r\n");
            while(1);
        }
        break;
    
    default:
        printk("Unknown exception\r\n");
        while(1);
        break;
    }
}