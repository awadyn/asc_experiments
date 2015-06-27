/*
"round", "pc", "|e|", "|y-x|", "|y-u|", "regret", "mse", "error", "round/s"
*/

{
    static double regret, hits;
    double mse, hit, tic, toc, dt, error;
    uint64_t counter;

    regret += loss;
    mse = h ? fann_get_MSE(h) : 0.25;
    hit = (loss == 0);
    hits += hit;
    error = 100 * (1 - hits / (t + 1));
    if (t == 0)
        tic = asc_util_timestamp();
    toc = asc_util_timestamp();
    dt = toc - tic;

    if (read(fd, &counter, sizeof(uint64_t)) != sizeof(uint64_t)) {
        printk("%s: read failed on fd `%d'", __func__, fd);
        assert(0);
    }

    static double oldR;

    double R = counter;
    double q = t + 1;

    R = MAX(1, counter - q);

    R = MAX(oldR + 1, R);

    oldR = R;

    printf("%7ld\t", t);
    printf("%06lx\t", asc_ssv_pc(y));
    printf("%.7s\t", asc_ssv_hash(y).sha[0]);
    printf("%7ld\t", asc_ssv_popcount(e));
    printf("%7ld\t", asc_ssv_popcount(z));
    printf("%7.0f\t", loss);
    printf("%7.1f\t", regret / (t + 1));
    printf("%7.4f\t", mse);
    printf(loss == 0 ? "%+7.2f\t" : "%7.2f\t", error);
    printf("%7.1e", R / dt);
}
