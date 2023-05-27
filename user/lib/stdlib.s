.section .text
.global writeu
.global sleepu
.global exitu
.global waitu
.global open_file
.global close_file
.global get_file_size

writeu:
    # Allocate 16 bytes on the stack to accomodate the args to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the args on the stack beforehand
    sub sp, sp, #16
    # Set the syscall index to 0 (write screen) in x8
    mov x8, #0
    stp x0, x1, [sp]
    # Load the arg count in x0
    mov x0, #2
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #16
    ret

sleepu:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 1 (sleep) in x8
    mov x8, #1
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret

exitu:
    # No arguments to this syscall hence no stack space required
    # Set the syscall index to 2 (exit) in x8
    mov x8, #2
    # Load the arg count in x0
    mov x0, #0
    # Operating system trap
    svc #0
    ret

waitu:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 3 (wait) in x8
    mov x8, #3
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret

open_file:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 4 (open file) in x8
    mov x8, #4
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret

close_file:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 5 (close file) in x8
    mov x8, #5
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret

get_file_size:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 6 (file size) in x8
    mov x8, #6
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret