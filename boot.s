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
