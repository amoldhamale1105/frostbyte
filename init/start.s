.section .text
.global _start

_start:
    bl main
# The following infinite loop in temporary until we implement the exit system call in the kernel
end:
    b end
