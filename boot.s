.section .text
.global _start

_start:
    mrs x0, mpidr_el1
    # get the lower 2 bits of the x0 register
    and x0, x0, #3
    # check if value if 0 (CPU0). We are building this kernel to run on CPU0 core only
    cmp x0, #0      
    beq kernel_entry

end:
    # TODO infinite loop with recursive calls. Could be done differently perhaps? 
    b end               

kernel_entry:
    # Get the current exception/execution level
    mrs x0, currentel
    lsr x0, x0, #2
    cmp x0, #2
    # If the exception level is not 2 in the beginning, branch to end
    bne end

    # Set system control register sctlr_el1 with 0 using the zero register xzr
    msr sctlr_el1, xzr
    # Set bit 31 of the hypervisor control register to enable Aarch64 mode
    mov x0, #1
    lsl x0, x0, #31
    msr hcr_el2, x0

    # Set the spsr register which will restore contents of pstate register with EL1 mode field and masked interrrupts (DAIF bits set to 1)
    mov x0, #0b1111000101
    msr spsr_el2, x0
    # Set elr register to el1_entry which restores the return address on returning from the exception
    # Get the address of e11_entry label in x0 and store it in elr register
    adr x0, el1_entry
    msr elr_el2, x0

    eret

el1_entry:
    # initialising stack pointer to 0x80000 which will then grow downwards from there
    mov sp, #0x80000
    # Load start address of bss in register x0 and end address in x1
    ldr x0, =bss_start
    ldr x1, =bss_end
    # Save size in x2 and value 0 in x1, call memset and initialize bss segment with 0
    sub x2, x1, x0
    mov x1, #0
    bl memset

    bl kmain
    # Nothing to do after control returns from kernel main
    b end               
