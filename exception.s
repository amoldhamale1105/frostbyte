.section .text
.global vector_table

# Align the vector table to a 2KB boundary (0x800 = 2048)
# Aligh each handler within it to 128 byte boundary (0x80 = 128)
.balign 0x800
vector_table:
# Current el with sp0 handlers for EL1
current_el_sp0_sync:
    # Branch to error because these handlers should never be called ideally
    b error

.balign 0x80
current_el_sp0_irq:
    b error

.balign 0x80
current_el_sp0_fiq:
    b error

.balign 0x80
current_el_sp0_serror:
    b error

# Current el with spn handlers for EL1. These are the ones we'll use for EL1 exceptions
.balign 0x80
current_el_spn_sync:
    b sync_handler

.balign 0x80
current_el_spn_irq:
    b error

.balign 0x80
current_el_spn_fiq:
    b error

.balign 0x80
current_el_spn_serror:
    b error

# Lower el with aarch64 handlers for EL0. These are the ones we'll use for EL0 exceptions
.balign 0x80
lower_el_aarch64_sync:
    b error

.balign 0x80
lower_el_aarch64_irq:
    b error

.balign 0x80
lower_el_aarch64_fiq:
    b error

.balign 0x80
lower_el_aarch64_serror:
    b error

# Lower el with aarch32 handlers for EL0
.balign 0x80
lower_el_aarch32_sync:
    b error

.balign 0x80
lower_el_aarch32_irq:
    b error

.balign 0x80
lower_el_aarch32_fiq:
    b error

.balign 0x80
lower_el_aarch32_serror:
    b error

sync_handler:
    # Allocate space for 32 registers on the stack for interrupted context. We have total of 31 GPRs
    sub sp, sp, #(32*8)
    # Use store pair instruction to save all 32 GPRs on the stack, 2 at a time
    stp x0, x1, [sp]
    # Move 16 bytes up in the stack to store the next pair (pair of 64 bit registers = 16 bytes) 
    stp x2, x3, [sp, #(16*1)]
    stp x4, x5, [sp, #(16*2)]
    stp x6, x7, [sp, #(16*3)]
    stp x8, x9, [sp, #(16*4)]
    stp x10, x11, [sp, #(16*5)]
    stp x12, x13, [sp, #(16*6)]
    stp x14, x15, [sp, #(16*7)]
    stp x16, x17, [sp, #(16*8)]
    stp x18, x19, [sp, #(16*9)]
    stp x20, x21, [sp, #(16*10)]
    stp x22, x23, [sp, #(16*11)]
    stp x24, x25, [sp, #(16*12)]
    stp x26, x27, [sp, #(16*13)]
    stp x28, x29, [sp, #(16*14)]
    str x30, [sp, #(16*15)]

    # Exception ID 1 means synchronous exception
    mov x0, #1
    # Use exception syndrome register value as second arg which holds information about the current exception
    mrs x1, esr_el1
    # Use the link register to as third argument which holds the return address 
    mrs x2, elr_el1
    bl handler

    # Restore GPRs of previous context with the load pair instruction
    ldp x0, x1, [sp]
    ldp x2, x3, [sp, #(16*1)]
    ldp x4, x5, [sp, #(16*2)]
    ldp x6, x7, [sp, #(16*3)]
    ldp x8, x9, [sp, #(16*4)]
    ldp x2, x3, [sp, #(16*1)]
    ldp x10, x11, [sp, #(16*5)]
    ldp x12, x13, [sp, #(16*6)]
    ldp x14, x15, [sp, #(16*7)]
    ldp x16, x17, [sp, #(16*8)]
    ldp x18, x19, [sp, #(16*9)]
    ldp x20, x21, [sp, #(16*10)]
    ldp x22, x23, [sp, #(16*11)]
    ldp x24, x25, [sp, #(16*12)]
    ldp x26, x27, [sp, #(16*13)]
    ldp x28, x29, [sp, #(16*14)]
    ldr x30, [sp, #(16*15)]

    add sp, sp, #(32*8)

    eret

error:
    # Allocate space for 32 registers on the stack for interrupted context. We have total of 31 GPRs
    sub sp, sp, #(32*8)
    # Use store pair instruction to save all 32 GPRs on the stack, 2 at a time
    stp x0, x1, [sp]
    # Move 16 bytes up in the stack to store the next pair (pair of 64 bit registers = 16 bytes) 
    stp x2, x3, [sp, #(16*1)]
    stp x4, x5, [sp, #(16*2)]
    stp x6, x7, [sp, #(16*3)]
    stp x8, x9, [sp, #(16*4)]
    stp x10, x11, [sp, #(16*5)]
    stp x12, x13, [sp, #(16*6)]
    stp x14, x15, [sp, #(16*7)]
    stp x16, x17, [sp, #(16*8)]
    stp x18, x19, [sp, #(16*9)]
    stp x20, x21, [sp, #(16*10)]
    stp x22, x23, [sp, #(16*11)]
    stp x24, x25, [sp, #(16*12)]
    stp x26, x27, [sp, #(16*13)]
    stp x28, x29, [sp, #(16*14)]
    str x30, [sp, #(16*15)]

    # 0 is unknown error. Pass it as an arg to the global exception handler, handler
    mov x0, #0
    # Global exception handler which accepts arg as the exception id
    bl handler

    # Restore GPRs of previous context with the load pair instruction
    ldp x0, x1, [sp]
    ldp x2, x3, [sp, #(16*1)]
    ldp x4, x5, [sp, #(16*2)]
    ldp x6, x7, [sp, #(16*3)]
    ldp x8, x9, [sp, #(16*4)]
    ldp x2, x3, [sp, #(16*1)]
    ldp x10, x11, [sp, #(16*5)]
    ldp x12, x13, [sp, #(16*6)]
    ldp x14, x15, [sp, #(16*7)]
    ldp x16, x17, [sp, #(16*8)]
    ldp x18, x19, [sp, #(16*9)]
    ldp x20, x21, [sp, #(16*10)]
    ldp x22, x23, [sp, #(16*11)]
    ldp x24, x25, [sp, #(16*12)]
    ldp x26, x27, [sp, #(16*13)]
    ldp x28, x29, [sp, #(16*14)]
    ldr x30, [sp, #(16*15)]

    add sp, sp, #(32*8)
    
    # Return from exception to the previous exception level (EL0)
    # This will load value of spsr register in pstate and elr register in program counter as return address
    eret
