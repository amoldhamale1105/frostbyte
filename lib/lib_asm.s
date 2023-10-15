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

.global delay
.global out_word
.global in_word
.global memset
.global memcpy
.global memmove
.global memcmp
.global get_el

get_el:
    # Read the currentel system register for current exception level (ELO-EL3) and save it in x0 register
    mrs x0, currentel
    # Right shift x0 by 2 bits because our data lies at bit 2 and 3
    lsr x0, x0, #2
    ret

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
    # If x1 is higher or same as x0, start copying from the first byte
    bhs copy
    # x3 = base address in x1 + size
    add x3, x1, x2
    cmp x3, x0
    # If x3 is lower or same as x0 meaning src and dst addresses do NOT overlap
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
