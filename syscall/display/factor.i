/*
"round", "pc", "|e|", "|y-x|", "|y-u|", "regret", "mse", "error", "round/s"
*/

{
    long dRBX, dRCX;

    dRBX = asc_ssv_qword(y, RBX) - asc_ssv_qword(x, RBX);
    dRCX = asc_ssv_qword(x, RCX) - asc_ssv_qword(y, RCX);

    fprintf(stderr, "%7ld\t", t);
    fprintf(stderr, "%06lx\t", asc_ssv_pc(y));
    fprintf(stderr, "%.7s\t", asc_ssv_hash(y).sha[0]);
    fprintf(stderr, "%15ld\t", dRBX);
    fprintf(stderr, "%15lx\t", dRBX);

    fprintf(stderr, "\n");
    fflush(stderr);
}
