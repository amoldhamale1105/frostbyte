.section .text
# Macro to save 31 GPRs on the stack
.macro save_context
    sub sp, sp, #(32*8)
    stp x0, x1, [sp]
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
.endm
# Macro to restore 31 GPRs from the stack in resepctive registers
.macro restore_context    
    ldp x0, x1, [sp]
    ldp x2, x3, [sp, #(16*1)]
    ldp x4, x5, [sp, #(16*2)]
    ldp x6, x7, [sp, #(16*3)]
    ldp x8, x9, [sp, #(16*4)]
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
.endm

.global memset
.global memcpy
.global memmove
.global memcmp

.global writeu
.global sleep
.global exit
.global wait
.global waitpid
.global open_file
.global close_file
.global get_file_size
.global read_file
.global fork
.global exec
.global getchar
.global getpid
.global read_root_dir
.global getppid
.global get_active_procs
.global get_proc_data
.global kill
.global signal

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

sleep:
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

exit:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 2 (exit) in x8
    mov x8, #2
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret

wait:
    # Move the first arg passed to x1 and set pid value to -1 in x0
    mov x1, x0
    mov x0, #1
    neg x0, x0
waitpid:
    # Allocate 16 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #16
    stp x0, x1, [sp]
    # Set the syscall index to 3 (wait) in x8
    mov x8, #3
    # Load the arg count in x0
    mov x0, #2
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #16
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
    # Allocate 16 bytes on the stack to accomodate the args to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the args on the stack beforehand
    sub sp, sp, #16
    stp x0, x1, [sp]
    # Set the syscall index to 9 (exec) in x8
    mov x8, #9
    # Load the arg count in x0
    mov x0, #2
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #16
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

getppid:
    # No arguments to this syscall hence no stack space required
    # Set the syscall index to 13 (get parent process ID) in x8
    mov x8, #13
    # Load the arg count in x0
    mov x0, #0
    # Operating system trap
    svc #0
    ret

get_active_procs:
    # Allocate 8 bytes on the stack to accomodate the argument to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the arg on the stack beforehand
    sub sp, sp, #8
    str x0, [sp]
    # Set the syscall index to 14 (active process ID list) in x8
    mov x8, #14
    # Load the arg count in x0
    mov x0, #1
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #8
    ret

get_proc_data:
    # Allocate 40 bytes on the stack to accomodate the args to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the args on the stack beforehand
    sub sp, sp, #40
    stp x0, x1, [sp]
    stp x2, x3, [sp, #16]
    str x4, [sp, #(16*2)]
    # Set the syscall index to 15 (get process data for given pid) in x8
    mov x8, #15
    # Load the arg count in x0
    mov x0, #5
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #40
    ret

kill:
    # Allocate 16 bytes on the stack to accomodate the args to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the args on the stack beforehand
    sub sp, sp, #16
    stp x0, x1, [sp]
    # Set the syscall index to 16 (send signal) in x8
    mov x8, #16
    # Load the arg count in x0
    mov x0, #2
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #16
    ret

signal:
    # Allocate 24 bytes on the stack to accomodate the args to this function
    # Note that in aarch64, args to functions are loaded in GPRs not the stack
    # We need the registers for other purposes hence saving the args on the stack beforehand
    sub sp, sp, #24
    stp x0, x1, [sp]
    # Pass third argument as signal handler proxy routine which will be invoked by the kernel
    ldr x2, =sighandler_proxy
    str x2, [sp, #16]
    # Set the syscall index to 17 (handle signal) in x8
    mov x8, #17
    # Load the arg count in x0
    mov x0, #3
    # Load x1 with the pointer to the arguments i.e. the current stack pointer
    mov x1, sp
    # Operating system trap
    svc #0

    # Restore the stack
    add sp, sp, #24
    ret

sighandler_proxy:
    save_context
    # Load signal value in x0 as arg to custom handler and custom handler address in x1 saved by the kernel on the stack
    ldp x0, x1, [sp, #(16*16)]
    # Branch from x1 register to user specified custom handler
    blr x1
    # Save register values of previous process context which are going to be overwritten during proxy request
    ldr x1, [sp, #8]
    ldr x8, [sp, #(16*4)]
    stp x1, x8, [sp, #(16*16)]
    restore_context
    # Set special request code (signal handler proxy restore) in x8
    mov x8, #101
    # Load x1 with the pointer to selective previous context data
    mov x1, sp
    # Operating system trap
    svc #0
    # We should never reach here. The process should resume execution at the point where it was previously interrupted
    ret