#ifndef HANDLER_H
#define HANDLER_H

#include "stdint.h"

struct ContextFrame
{
    int64_t x0;
    int64_t x1;
    int64_t x2;
    int64_t x3;
    int64_t x4;
    int64_t x5;
    int64_t x6;
    int64_t x7;
    int64_t x8;
    int64_t x9;
    int64_t x10;
    int64_t x11;
    int64_t x12;
    int64_t x13;
    int64_t x14;
    int64_t x15;
    int64_t x16;
    int64_t x17;
    int64_t x18;
    int64_t x19;
    int64_t x20;
    int64_t x21;
    int64_t x22;
    int64_t x23;
    int64_t x24;
    int64_t x25;
    int64_t x26;
    int64_t x27;
    int64_t x28;
    int64_t x29;
    int64_t x30;
    int64_t sp0;
    int64_t trapno;
    int64_t esr;
    int64_t elr;
    int64_t spsr;
};

#define PSTATE_MODE_MASK 0xF /* The mode field bitmask (EL0, EL1 etc.) of pstate register */

void init_timer(void);
void enable_irq(void);
void init_interrupt_controller(void);
uint64_t get_ticks(void);

#endif