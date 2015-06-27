/* A simple C runtime bootstrap.  */

.globl _start

_start:
    /* Required by the ABI.  */
    xor %rbp, %rbp

    /* Get argc and argv.  */
    pop %rdi
    mov %rsp, %rsi

    /* Align stack pointer as per ABI.  */
    and $0xfffffffffffffff0, %rsp

    /* Jump into C entry point.  */
    call main

    /* Program return code.  */
    mov %rax, %rdi

    /* Exit.  */
    mov $0x3c, %rax
    syscall

    /* We should not be here.  */
    hlt
