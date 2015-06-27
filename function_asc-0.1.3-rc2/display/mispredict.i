/*
"round", "pc", "|e|", "|y-x|", "|y-u|", "regret", "mse", "error", "round/s"
*/

{
    mp_bitcnt_t j;

    if (asc_ssv_popcount(r) < 100) {
        for (j = asc_ssv_scan1(r,0); j < ULONG_MAX; j = asc_ssv_scan1(r, j+64))
        {
            if (j/64 == RBX)
                printf("%7ld\t%7ld\t[RBX]\n", t, j / 64);
            else if (j/64 == RAX)
                printf("%7ld\t%7ld\t[RAX]\n", t, j / 64);
            else if (j/64 == RDX)
                printf("%7ld\t%7ld\t[RDX]\n", t, j / 64);
            else if (j/64 == EFLAGS)
                printf("%7ld\t%7ld\t[EFLAGS]\n", t, j / 64);
            else
                printf("%7ld\t%7ld\n", t, j / 64);
        }
        printf("\n");
        fflush(stdout);
    }
}
