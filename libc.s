.global delay
.global out_word
.global in_word

delay:
    # First arg will be present in register x0 which will be subtracted till it becomes 0
    subs x0, x0, #1
    bne delay
    ret

out_word:
    # Store value in register w1 (32-bit segment of 64-bit register x1) to memory location pointed by register x0
    # Note that according to ARM64 calling conventions, x0 contains first param and x1 second to a procedure
    str w1, [x0]
    ret

in_word:
    # Load contents of memory location x0 into 32-bit register w0
    ldr w0, [x0]
    ret
    