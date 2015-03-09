#include <asc.h>
#include <sys/reg.h>

long integrate(mpz_t y, __attribute((unused)) mpz_t const x,
               struct machine *m, enum mode o)
{
    long c;

    /* Execute forward according to mode.  */
    if ((c = transition(m, o)) == 0)
        goto failure;

    /* Get representation of new state vector.  */
    if (peek(y, m) < 0)
        goto failure;

    return c;

  failure:
    return 0;
}
