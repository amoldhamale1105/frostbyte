#ifndef IRQ_H
#define IRQ_H

#include "memory.h"

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