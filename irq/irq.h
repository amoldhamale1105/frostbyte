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

#define CNTP_EL0        TO_VIRT(0x40000040) /* Core 0 interrupt timer control register */ 
#define CNTP_STATUS_EL0 TO_VIRT(0x40000060) /* Core 0 interrupt source register */

#define BASE_ADDR TO_VIRT(0x3f000000)

#define IRQ_BASIC_PENDING       (BASE_ADDR + 0xb200)
#define ENABLE_IRQS_1           (BASE_ADDR + 0xb210) /* IRQ 0-31 */
#define ENABLE_IRQS_2           (BASE_ADDR + 0xb214) /* IRQ 32-63 */
#define ENABLE_BASIC_IRQS       (BASE_ADDR + 0xb218)
#define DISABLE_IRQS_1          (BASE_ADDR + 0xb21c)
#define DISABLE_IRQS_2          (BASE_ADDR + 0xb220)
#define DISABLE_BASIC_IRQS      (BASE_ADDR + 0xb224)

#endif