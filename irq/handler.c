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

#include <stdint.h>
#include <io/print.h>
#include <lib/lib.h>
#include <irq/irq.h>
#include <io/uart.h>
#include <irq/syscall.h>
#include <process/process.h>
#include "handler.h"

void enable_timer(void);
uint32_t read_timer_status(void);
void set_timer_interval(uint32_t value);
uint32_t read_timer_freq(void);

static uint32_t timer_interval = 0;
static uint64_t ticks = 0;

void init_interrupt_controller(void)
{
#ifdef RPI4
    /* Disable the distributor and CPU interface before configuring the interrupt controller */
    out_word(DISTR_CTL, 0);
    out_word(CPUIF_CTL, 0);
    /* GIC400 supports 256 priority levels (0-255). Higher the number, lower the priority.
       Setting 255 means that all interrupts are allowed to be processed by the CPU interface */
    out_word(ICC_PR, 0xff);
    /* The distributor priority register is 32 bits long and can hold priority levels for 4 interrupts (1 byte each)
       Timer interrupt will use the 17th ICD register (16 * 4-byte ICD_PR = 64). Make it the highest priority interrupt (value 0) */
    out_word(ICD_PR + TIMER_IRQ, 0);
    /* UART interrrupt will use the 38th ICD register (38 * 4-byte ICD_PR = ~153) */
    out_word(ICD_PR + ((VC_IRQ_BASE + UART_IRQ)/4) * 4, 1);
    /* The processor target interrupt register has the same structure and calculation as ICD_PR hence offset for timer IRQ remains the same
       This register determines which CPU core handles a given interrupt at specified offset. Timer IRQ is configured to be handled by core 0 */
    out_word(ICD_PTR + TIMER_IRQ, 1);
    /* Configure core 0 to habdle UART interrupt by setting lowest bit of 2nd byte in the 38th ICD register */
    out_word(ICD_PTR + ((VC_IRQ_BASE + UART_IRQ)/4) * 4, 0x100);
    /* Interrupt config register holds sensitivity data for 16 interrupts 2-bit each. Timer IRQ's offset will fall in 4th register.
       The secomd bit of each entry: 0 => level triggered, 1 => edge triggered, We set timer interrupt to be edge triggered */
    out_word(ICD_ICFGR + TIMER_IRQ/4, 2);
    /* UART IRQ will fall in the 10th register so register offset will be 9 * 4-byte ICD_ICFGR = 36
       Write 1 to the upper bit of the 9th 2-bit block of this register ((96+57) - (16*9) = 9) to configure it as edge triggered */
    out_word(ICD_ICFGR + ((VC_IRQ_BASE + UART_IRQ)/16) * 4, 0x20000);
    /* Each bit of 4-byte interrupt set enable register determines whether an interrupt is enabled or not
       Timer interrupt will use the 3rd ICD register hence offset will be 8 (2 * 4-byte ICD_ISENABLE = 8) */
    out_word(ICD_ISENABLE + TIMER_IRQ/8, 1);
    /* Calculate register offset for UART IRQ and write 1 to 25th bit in it to enable this interrupt */
    out_word(ICD_ISENABLE + ((VC_IRQ_BASE + UART_IRQ)/32) * 4, (1 << 25));
    /* Enable the distributor and CPU interface */
    out_word(DISTR_CTL, 1);
    out_word(CPUIF_CTL, 1);
#else
    /* Disable all interrupts initially to enable selectively later on */
    out_word(DISABLE_BASIC_IRQS, 0xffffffff);
    out_word(DISABLE_IRQS_1, 0xffffffff);
    out_word(DISABLE_IRQS_2, 0xffffffff);

    /* Set bit 25 in intr enable register 2 to enable IRQ 57 of UART */
    out_word(ENABLE_IRQS_2, (1 << 25));
#endif
}

/* Tick interval = 10 ms */
uint64_t get_ticks(void)
{
    return ticks;
}

void init_timer(void)
{
#ifdef RPI4
    /* timer_clock = apb_clock / (pre-divider+1) => 250 MHz / (0x7d+1) = ~2 MHz, 250 and 7d are default values of apb_clock and pre-divider */
    out_word(TIMER_PREDIV, 0x7d);
    /* The exact timer_clock value from the previous equation was 1984126 Hz i.e. 1984126 ticks per second
       Dividing this by 100 will yield the value of timer_clock elapsed after every 10 ms which is the kernel's tick interval */
    out_word(TIMER_LOAD, 19841);
    /* With ref to BCM2711 TRM, high bit 1 => 32-bit counter, bit 5 => timer intr enabled, bit 7 => timer enabled */
    /* The timer interrupt is configured as a recurring interrupt which is fired every 10 ms */
    out_word(TIMER_CTL, 0b10100010);
#else
    /* Save the timer interval for the handler to retrigger when the interrupt is triggered for the first time */
    timer_interval = read_timer_freq() / 100;
    enable_timer();
    /* Set bit 1 (IRQ enable) of the core timer status register to enable timer interrupts */
    out_word(CNTP_EL0, (1 << 1));
#endif
}

static void timer_interrupt_handler(void)
{
#ifdef RPI4
    uint32_t status = in_word(TIMER_MSKIRQ);
#else
    uint32_t status = read_timer_status();
#endif
#ifdef RPI4
    /* If bit 0 of mask register is set, timer interrupt is asserted */
    if (status & 1)
#else
    /* If bit 2 is set, it means the timer has fired. Reset the timer with the same interval */
    if (status & (1 << 2))
#endif
    {
        ticks++;
        wake_up(SLEEP_SYSCALL);
#ifdef RPI4
        /* Acknowledge the interrupt by clearing the pending bit of the timer ack register */
        out_word(TIMER_ACK, 1);
#else
        set_timer_interval(timer_interval);
#endif
    }
}

static uint32_t get_irq_number(void)
{
#ifdef RPI4
    /* Read the IRQ number from the interrupt ack register of the interrupt controller */
    return in_word(ICC_ACK);
#else
    /* Read the basic pending register to get the asserted IRQ number */
    return in_word(IRQ_BASIC_PENDING);
#endif
}

void handler(struct ContextFrame* ctx)
{
    uint32_t irq;
    /* Whether the exception occured because of a userspace process */
    bool user_except = ((ctx->spsr & PSTATE_MODE_MASK) == 0);
    struct Process* curr_proc = get_curr_process();

    /* Save register context for idle process from the kernel stack */
    if (curr_proc->pid == 0)
        curr_proc->reg_context = ctx;

    switch (ctx->trapno)
    {
    case 1:
        if (user_except){
            printk("%x: Process (PID %d) resulted in a synchronous exception. Terminating\n", ctx->elr, curr_proc->pid);
            /* Although this exit call occurs in kernel space, it is meant to terminate the current user process which caused this exception */
            exit(curr_proc, 1, false);
        }
        else{
            printk("Sync exception at %x: %x\r\n", ctx->elr, ctx->esr);
            while(1);
        }
        break;
    /* Hardware interrupt */
    case 2:
#ifdef RPI4
        irq = get_irq_number();
        if (irq == TIMER_IRQ)
#else
        /* Read the interrupt source register to check what kind of hardware interrupt it is */
        irq = in_word(CNTP_STATUS_EL0);
        /* High bit 1 indicates timer interrupt */
        if (irq & (1 << 1))
#endif
        {
            timer_interrupt_handler();
            trigger_scheduler();
        }
        else{
#ifdef RPI4
            if (irq == (VC_IRQ_BASE + UART_IRQ))
#else
            irq = get_irq_number();
            /* Bit 19 of the interrupt pending register is for IRQ 57 i.e. UART interrupt */
            if (irq & (1 << 19))
#endif
                uart_handler();
            else{
                printk("Unknown hardware interrupt\r\n");
                while(1);
            }
        }
#ifdef RPI4
        /* Send end of interrupt to the controller, indicating that the interrupt handling is over by writing irq number to it */
        out_word(ICC_EOI, irq);
#endif
        break;
    /* System call trap */
    case 3:
        system_call(ctx);
        break;
    default:
        if (user_except){
            printk("%x: Process (PID %d) resulted in an unknown exception. Terminating\n", ctx->elr, curr_proc->pid);
            exit(curr_proc, 1, false);
        }
        else{
            printk("Unknown exception at %x: %x\r\n", ctx->elr, ctx->esr);
            while(1);
        }
        break;
    }
}