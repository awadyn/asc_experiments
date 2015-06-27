#include <asc.h>
#include <elf.h>
#include <asm/unistd_64.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/uio.h>
#include <sys/user.h>

int peek(mpz_t x, struct machine *m)
{
    struct iovec gpr, fpr, mem;
    unsigned long c;

    mpz_setbit(x, m->dim - 1);

    /* Reset error number.  */
    errno = 0;

    /* Initialize descriptor for general-purpose registers.  */
    gpr = m->io[0];
    gpr.iov_base = (char *) x->_mp_d;

    /* Fetch general-purpose registers.  */
    if (ptrace(PTRACE_GETREGSET, m->pid, NT_PRSTATUS, &gpr) < 0) {
        diagnostic("ptrace getregset NT_PRSTATUS");
        goto failure;
    }

    /* Bail out if unexpected number of bytes.  */
    if (gpr.iov_len != sizeof(struct user_regs_struct)) {
        diagnostic("panic: got %ld bytes", gpr.iov_len);
        goto failure;
    }

    /* Reset RF flag so we get consistent SHA-256 hashes.  */
    mpz_clrbit(x, 64 * EFLAGS + 16);

    /* Reset psuedo-register so we get consistent SHA-256 hashes.  */
    if (x->_mp_d[ORIG_RAX] == __NR_exit)
        x->_mp_d[R13] = 0;

    /* Initialize descriptor for floating point and vector registers.  */
    fpr = m->io[1];
    fpr.iov_base = (char *) x->_mp_d + gpr.iov_len;

    /* Fetch floating point and vector registers.  */
    if (ptrace(PTRACE_GETREGSET, m->pid, NT_X86_XSTATE, &fpr) < 0) {
        diagnostic("ptrace getregset NT_X86_XSTATE");
        goto failure;
    }

    /* Bail out if we got unexpected number of bytes.  */
    if (fpr.iov_len != sizeof(struct user_fpregs_struct) + 320) {
        diagnostic("panic: got %ld bytes", fpr.iov_len);
        goto failure;
    }

    /* Set up local scatter/gather I/O descriptor.  */
    mem.iov_base = (unsigned char *) x->_mp_d + gpr.iov_len + fpr.iov_len;
    mem.iov_len = m->dim / 8 - (gpr.iov_len + fpr.iov_len);

    c = process_vm_readv(m->pid, &mem, 1, m->io+2, HASH_COUNT(m->maps)-2, 0);

    if (c != mem.iov_len) {
        diagnostic("process_vm_readv");
        goto failure;
    }

    if (errno) {
        diagnostic("error");
        goto failure;
    }

    return m->dim;

  failure:
    return -1;
}
