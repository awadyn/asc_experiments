/* A simple decrementing counter kernel.  */

.globl _start

_start:
    /* Default loop count.  */
    mov $27, %rcx

    /* Check for command line argument.  */
    cmpq $0x1, 0x0(%rsp)
    je .L1

    /* Use higher loop count if so.  */
    movq $0xefffffffffffffff, %rcx

    /* Main loop.  */
.L1:
    loop .L1

    /* Set return code.  */
    mov $0, %rdi

    /* Exit.  */
    mov $0x3c, %rax
    syscall

    /* Should not be here.  */
    hlt
