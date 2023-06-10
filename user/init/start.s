.section .text
.global _start

_start:
    bl main
    # Call exit syscall once the main function returns for the process resource cleanup
    bl exit
