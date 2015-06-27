/*
"round", "pc", "|e|", "|y-x|", "|y-u|", "regret", "mse", "error", "round/s"
*/

{
    static double regret, hits;
    double mse, hit, toc, dt, error, mips, mu;
    uint64_t counter;

    if (t > 3)
        regret += loss;
    mse = h ? fann_get_MSE(h) : 0.25;
    hit = (loss == 0);
    hits += hit;
    error = 100 * (1 - hits / (t + 1));
    toc = asc_util_timestamp();
    dt = toc - tic;
    mu = regret / (t + 1);

    if (read(fd[0], &counter, sizeof(uint64_t)) != sizeof(uint64_t)) {
        assert(0);
    }

    static double oldR;

    double R = counter;
    double q = t + 1;

    R = MAX(1, counter - q);

    R = MAX(oldR + 1, R);

    oldR = R;

    mips = ((R + Payoff) / 1e6) / dt;

    fprintf(stderr, "%7ld\t", t);
    fprintf(stderr, "%06lx\t", asc_ssv_pc(y));
    fprintf(stderr, "%.7s\t", asc_ssv_hash(y).sha[0]);
    fprintf(stderr, "%7ld\t", asc_ssv_popcount(e));
    fprintf(stderr, "%7ld\t", asc_ssv_popcount(z));
    fprintf(stderr, "%7.0f\t", loss);

    if (mu < 1e5)
        fprintf(stderr, "%7.1f\t", mu);
    else
        fprintf(stderr, "%7.1e\t", mu);

    fprintf(stderr, "%7.4f\t", mse);

    fprintf(stderr, loss == 0 ? "%+7.2f\t" : "%7.2f\t", error);

    if (mips < 1)
        fprintf(stderr, "%7.5f", mips);
    else if (mips >= 10000)
        fprintf(stderr, "%7.1f", mips);
    else
        fprintf(stderr, "%7.2f", mips);

    fprintf(stderr, "\n");
    fflush(stderr);
}
