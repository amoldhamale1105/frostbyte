.section .text
.global _start

_start:
    bl main
    # Here, the return value from main stored in x0 will be used as first arg (exit status) to exit
    bl exit
