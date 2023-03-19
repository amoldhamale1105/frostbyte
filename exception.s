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
    # Exception ID 1 means synchronous exception
    mov x0, #1
    # Use exception syndrome register value as second arg which holds information about the current exception
    mrs x1, esr_el1
    # Use the link register to as third argument which holds the return address 
    mrs x2, elr_el1
    bl handler

    eret

error:
    # 0 is unknown error. Pass it as an arg to the global exception handler, handler
    mov x0, #0
    # Global exception handler which accepts arg as the exception id
    bl handler
    # Return from exception to the previous exception level (EL0)
    # This will load value of spsr register in pstate and elr register in program counter as return address
    eret
