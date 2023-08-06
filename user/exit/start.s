.section .text
.global _start

_start:
    # Copy first arg to the main function from x2 to x0. Refer to exec function for rationale
    mov x0, x2
    bl main
    # Here, the return value from main stored in x0 will be used as first arg (exit status) to exit
    bl exit
