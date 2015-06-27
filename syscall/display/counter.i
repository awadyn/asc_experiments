{
    printf("%23lu\t", asc_ssv_qword(x, RCX) - asc_ssv_qword(y, RCX));
    printf("%23lx\t", asc_ssv_qword(x, RCX) - asc_ssv_qword(y, RCX));
    printf("\n");
    fflush(stdout);
}
