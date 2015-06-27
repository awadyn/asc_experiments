{
    printf("%7ld\t", t);
    printf("%15lu\t", asc_ssv_qword(x, R12));
    printf("%15lu\t", asc_ssv_qword(y, R12));
    printf("%15lu\t", asc_ssv_qword(y, R12) - asc_ssv_qword(x, R12));
    printf("%15lx\t", asc_ssv_qword(y, R12) - asc_ssv_qword(x, R12));
    printf("\n");
    fflush(stdout);
}
