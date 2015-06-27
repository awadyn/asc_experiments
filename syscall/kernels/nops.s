/* Perhaps the simplest possible kernel.  */

.globl _start

_start:
    /* Execute straight-line no-ops.  */
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    cmpq $0x2, 0x0(%rsp)
    je _start

    /* Set return code.  */
    mov $0, %rdi

    /* Exit.  */
    mov $0x3c, %rax
    syscall

    /* Should not be here.  */
    hlt
