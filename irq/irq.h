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

#ifndef IRQ_H
#define IRQ_H

#include <memory/memory.h>

#ifdef RPI4
#define GIC_BASE            TO_VIRT(0xff840000) /* GIC400 interrupt controller base register */
#define BASE_ADDR           TO_VIRT(0xfe000000) /* Timer base register */
#else
#define BASE_ADDR           TO_VIRT(0x3f000000) /* Interrupt controller base register */
#endif

#ifdef RPI4
#define DISTR_CTL           GIC_BASE + 0x1000   /* Distributor control register */
#define ICD_TYPE            DISTR_CTL + 0x4
#define ICD_ISENABLE        DISTR_CTL + 0x100   /* Interrupt set enable register */
#define ICD_ICENABLE        DISTR_CTL + 0x180
#define ICD_SET_PENDING     DISTR_CTL + 0x200
#define ICD_SET_ACTIVE      DISTR_CTL + 0x300
#define ICD_PR              DISTR_CTL + 0x400   /* Distributor priority register */
#define ICD_PTR             DISTR_CTL + 0x800   /* Processor target interrupt */
#define ICD_GROUP           DISTR_CTL + 0x80
#define ICD_ICFGR           DISTR_CTL + 0xc00   /* Interrupt config register */

#define CPUIF_CTL           GIC_BASE + 0x2000   /* CPU interface control register */
#define ICC_PR              CPUIF_CTL + 0x4     /* CPU interface interrupt priority register */
#define ICC_ACK             CPUIF_CTL + 0xc     /* Interrupt acknowledge register */
#define ICC_EOI             CPUIF_CTL + 0x10    /* End of interrupt register */
#else
#define IRQ_BASIC_PENDING       (BASE_ADDR + 0xb200)
#define ENABLE_IRQS_1           (BASE_ADDR + 0xb210) /* IRQ 0-31 */
#define ENABLE_IRQS_2           (BASE_ADDR + 0xb214) /* IRQ 32-63 */
#define ENABLE_BASIC_IRQS       (BASE_ADDR + 0xb218)
#define DISABLE_IRQS_1          (BASE_ADDR + 0xb21c)
#define DISABLE_IRQS_2          (BASE_ADDR + 0xb220)
#define DISABLE_BASIC_IRQS      (BASE_ADDR + 0xb224)
#endif

#ifdef RPI4
#define TIMER_IRQ           64                      /* ARM timer part of ARMC peripheral IRQs SPI IDs 64-79 */
#define TIMER_LOAD          (BASE_ADDR + 0xB400)    /* Timer value load register */
#define TIMER_CTL           (BASE_ADDR + 0xB408)    /* Timer control register */
#define TIMER_ACK           (BASE_ADDR + 0xB40C)    /* Timer acknowledge register */
#define TIMER_MSKIRQ        (BASE_ADDR + 0xB414)    /* Timer mask register */
#define TIMER_PREDIV        (BASE_ADDR + 0xB41c)    /* Timer pre-divider register */
#else
#define CNTP_EL0        TO_VIRT(0x40000040) /* Core 0 interrupt timer control register */ 
#define CNTP_STATUS_EL0 TO_VIRT(0x40000060) /* Core 0 interrupt source register */
#endif

#endif