.section .text
.global memset
.global memcpy
.global memmove
.global memcmp

.global writeu
.global sleepu
.global exitu
.global waitu
.global open_file
.global close_file
.global get_file_size
.global read_file
.global fork
.global exec
.global getchar
.global getpid
.global read_root_dir

memset:
    # x0 => dst x1 => value x2 => size
    cmp x2, #0
    beq memset_end
set:
    # Increment x0 (address of dst) by 1 after we store 1-byte value from w1 register to x0
    strb w1, [x0], #1
    # Subtract 1 from x2 and update conditional flags according to result (accomplished by suffix 's' in subs instruction)
    subs x2, x2, #1
    bne set

memset_end:
    ret

memcmp:
    # x0 => src1 x1 => src2 x2 => size
    mov x3, x0
    # We do this to clear the x0 register for possible return value
    mov x0, #0
compare:
    cmp x2, #0
    beq memcmp_end
    # Register x3 will increment by 1 afer we load the value in w4
    ldrb w4, [x3], #1
    ldrb w5, [x1], #1
    sub x2, x2, #1
    cmp w4, w5
    beq compare
    # Comparison failed. Return 1
    mov x0, #1

memcmp_end:
    ret

memmove:
memcpy:
    # x0 => dst x1 => src x2 => size
    cmp x2, #0
    beq memcpy_end
    # Temp value used to determine direction of traversal while copying data
    mov x4, #1

    cmp x1, x0
    # If x1 is greater than or equal to x0
    bhs copy
    # x3 = base address in x1 + size
    add x3, x1, x2
    cmp x3, x0
    # If x3 is less than or equal to x0 meaning src and dst addresses do NOT overlap
    bls copy

overlap:
    # Set src and dst to point to the last index (size - 1) of the memory location
    sub x3, x2, #1
    add x0, x0, x3
    add x1, x1, x3
    # negate x4 such that if it holds 1, new value will be -1
    # This is done to copy data backwards from the last index
    neg x4, x4

copy:
    # Load 1 byte data from memory address in x1 in w3
    ldrb w3, [x1]
    # Store 1 byte data from w3 in memory address in x0
    strb w3, [x0]
    add x0, x0, x4
    add x1, x1, x4
    subs x2, x2, #1
    # If size is not zero, continue copying
    bne copy

memcpy_end:
    ret

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

read_file:
    # Allocate 24 bytes on the stack to accomodate the args to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the args on the stack beforehand
    sub sp, sp, #24
    stp x0, x1, [sp]
    str x2, [sp, #16]
    # Set the syscall index to 7 (read file) in x8
    mov x8, #7
    # Load the arg count in x0
    mov x0, #3
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #24
    ret

fork:
    # No arguments to this syscall hence no stack space required
    # Set the syscall index to 8 (fork) in x8
    mov x8, #8
    # Load the arg count in x0
    mov x0, #0
    # Operating system trap
    svc #0
    ret

exec:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 9 (exec) in x8
    mov x8, #9
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret

getchar:
    # No arguments to this syscall hence no stack space required
    # Set the syscall index to 10 (get pressed key) in x8
    mov x8, #10
    # Load the arg count in x0
    mov x0, #0
    # Operating system trap
    svc #0
    ret

getpid:
    # No arguments to this syscall hence no stack space required
    # Set the syscall index to 11 (get process ID) in x8
    mov x8, #11
    # Load the arg count in x0
    mov x0, #0
    # Operating system trap
    svc #0
    ret

read_root_dir:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 12 (read root directory table) in x8
    mov x8, #12
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret
