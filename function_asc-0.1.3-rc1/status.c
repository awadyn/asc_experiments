#include <asc.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/reg.h>

int status(long r, mpz_t x, mpz_t y, mpz_t u, mpz_t d, mpz_t e,
           unsigned char *b, long a, struct machine *m)
{
    char *str, *buf;
    unsigned long h;
    long i, j, k, l;

    k = mpz_popcount(e);

    if (strcmp(m->metrics, "bits") == 0) {

        for (j = 0; j < m->dim; j++)
            printf("%ld\t%ld\t%x\n", r, j, mpz_tstbit(y, j));
        printf("\n");

        goto success;

    } else if (strcmp(m->metrics, "diverge") == 0) {
        /* Print header to standard output if necessary.  */
        if (r == 0) {
            printf("%7s\t%7s\t%15s\t%15s\t%16s\n",
                   "round", "feature", "y", "yhat", "error");
        }

        for (j = mpz_scan1(e, 0), i = 0; j > 0; j = mpz_scan1(e, j + 64), i++)
            printf("%7ld\t%7ld\t%15ld\t%15ld\t%16ld\n", r, j / 64,
                    y->_mp_d[j / 64], u->_mp_d[j / 64],
                    y->_mp_d[j / 64] - u->_mp_d[j / 64]);
        printf("\n");

        goto success;

    } else if (strcmp(m->metrics, "transition") == 0) {

        for (j = mpz_scan1(e, 0); j > 0; j = mpz_scan1(e, j + 1))
            printf("%7d\t", mpz_tstbit(x, j));

        for (j = mpz_scan1(e, 0); j > 0; j = mpz_scan1(e, j + 1))
            printf("%7d\t", mpz_tstbit(d, j));

        if (k > 0)
            printf("\n");

        goto success;

    } else if (strcmp(m->metrics, "qwords") == 0) {

        for (j = 0; j < m->dim; j += 64)
            printf("%ld\t%ld\t%016lx\n", r, j / 64, y->_mp_d[j / 64]);
        printf("\n");

        goto success;
    }

    /* Print header if necessary.  */
    if (r == 0 && index(m->metrics, ',')) {

        buf = strdupa(m->metrics);

        for (str = strtok(buf, ","); str; str = strtok(0, ","))
            printf("%7s\t", str);
        printf("\n");
    }

    buf = strdupa(m->metrics);

    l = mpz_hamdist(y, u);

    for (str = strtok(buf, ","); str; str = strtok(0, ",")) {
        if (strcmp(str, "bitrate") == 0) {
            printf("%7.3f\t", m->Bits / (r + 1.0));
        } else if (strcmp(str, "delta") == 0) {
            printf("%7ld\t", mpz_hamdist(y, x));
        } else if (strcmp(str, "drbx") == 0) {
            printf("%7lx\t", y->_mp_d[RBX] - x->_mp_d[RBX]);
        } else if (strcmp(str, "eflags") == 0) {
            for (j = 0; j < 64; j++)
                printf("%d", mpz_tstbit(y, EFLAGS * 64 + j));
        } else if (strcmp(str, "energy") == 0) {
            printf("%7ld\t", mpz_popcount(y));
        } else if (strcmp(str, "error") == 0) {
            if (l == 0)
                printf("%+7.3f\t", 100.0 - 100.0 * a / (r + 1));
            else
                printf("%7.3f\t", 100.0 - 100.0 * a / (r + 1));
        } else if (strcmp(str, "excited") == 0) {
            printf("%7ld\t", k);
        } else if (strcmp(str, "hash") == 0) {
            h = sha(0, b, y) >> (64 - 7 * 4);
            printf("%07lx\t", h);
        } else if (strcmp(str, "mips") == 0) {
            double dt = timestamp(0) - m->timestamp;
            double mips = dt > 0 ? (m->retired / 1e6) / dt : 0;
            printf("%7.2f\t", mips);
        } else if (strcmp(str, "retired") == 0) {
            if (log10(m->retired) < 7)
                printf("%7.0f\t", m->retired);
            else
                printf("%7.1e\t", m->retired);
        } else if (strcmp(str, "rax") == 0) {
            printf("%7ld\t", y->_mp_d[RAX]);
        } else if (strcmp(str, "rbx") == 0) {
            printf("%7ld\t", y->_mp_d[RBX]);
        } else if (strcmp(str, "rcx") == 0) {
            printf("%7ld\t", y->_mp_d[RCX]);
        } else if (strcmp(str, "rdi") == 0) {
            printf("%7ld\t", y->_mp_d[RDI]);
        } else if (strcmp(str, "rip") == 0 || strcmp(str, "pc") == 0) {
            printf(" %06lx\t", y->_mp_d[RIP]);
        } else if (strcmp(str, "round") == 0) {
            printf("%7ld\t", r);
        } else if (strcmp(str, "wrong") == 0) {
            printf("%7ld\t", l);
        } else {
            printf("\n");
            diagnostic("invalid telemetry: %s", str);
            goto failure;
        }
    }

    printf("\n");

  success:
    fflush(stdout);
    return 0;

  failure:
    return -1;
}
