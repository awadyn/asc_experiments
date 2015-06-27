/* Dense region of sparse state vector.  */
typedef struct ssv_subvector {
    struct iovec iov;
    int (*peek)(struct iovec *local, struct iovec *remote, pid_t pid);
    int (*poke)(struct iovec *local, struct iovec *remote, pid_t pid);
    mpz_t z;
} ssv_sub_t;

/* Sparse state vector.  */
typedef struct ssv {
    long N;
    struct {
        uint8_t SHA[256];
        char sha[1][8];
        long uid;
    };
    ssv_sub_t sub[];
} ssv_t;

#define ASC_TRACE 0

/* Error message macro.  */
#define printk(...)                                       \
do {                                                      \
    error_at_line(0, 0, __FILE__, __LINE__, __VA_ARGS__); \
} while (0)

/* Alias for error message macro useful when debugging.  */
#define qq printk

#define tr() printk("%s", __func__)

#define INFTY ULONG_MAX 

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[]);

int asc_help(void);
ssv_t *asc_ssv_calloc(long N);
ssv_t *asc_ssv_free(ssv_t *x);
pid_t asc_ptrace_start(int argc, char *argv[]);
ssv_t *asc_ssv_gather(ssv_t *x, pid_t pid);
int asc_peek_gpr(struct iovec *local, struct iovec *remote, pid_t pid);
int asc_poke_gpr(struct iovec *local, struct iovec *remote, pid_t pid);
int asc_peek_fpr(struct iovec *local, struct iovec *remote, pid_t pid);
int asc_poke_fpr(struct iovec *local, struct iovec *remote, pid_t pid);
int asc_peek_region(struct iovec *local, struct iovec *remote, pid_t pid);
int asc_poke_region(struct iovec *local, struct iovec *remote, pid_t pid);
ssv_t *asc_ssv_bind(ssv_t *x, pid_t pid);
double asc_timestamp(void);
long asc_ssv_qword(ssv_t const *x, long i);
long asc_ssv_pc(ssv_t const *x);
int asc_ssv_swap(ssv_t *x, ssv_t *y);
int asc_maps_parse(struct iovec *iov, mode_t *mode, char **name,
                   int M, pid_t pid);
ssv_t *asc_ssv_xor(ssv_t *z, ssv_t const *x, ssv_t const *y);
long asc_ssv_popcount(ssv_t const *x);
int asc_perf_attach(struct perf_event_attr *pe, pid_t pid);
int asc_perf_read(uint64_t *counter, int fd);
int asc_ptrace_breakpoint(pid_t pid, void *addr, double period,
                          struct perf_event_attr *pe, int fd);
mp_bitcnt_t asc_ssv_scan1(ssv_t const *x, mp_bitcnt_t j);
int asc_ssv_frees(ssv_t *X[]);
long asc_ssv_cmp(ssv_t const *x, ssv_t const *y);
mp_bitcnt_t asc_ssv_hi(ssv_t const *x, int i);
ssv_t *asc_ssv_set(ssv_t *x, ssv_t const *y);
int asc_ptrace_transition(pid_t pid, void *addr, double period, int fd);
ssv_t asc_ssv_hash(ssv_t const *x);
int asc_ptrace_blockstep(pid_t pid, int *status);
int asc_ptrace_syscall(pid_t pid, int status);
int asc_emit_pretty(pid_t pid[], ssv_t *X[], long L[], double A[]);
ssv_t *asc_ssv_ior(ssv_t *z, ssv_t const *x, ssv_t const *y);
int asc_event(pid_t pid[], ssv_t *X[], long L[], double A[], struct fann *h[],
              int (*f[])(pid_t [], ssv_t *[], long [], double []));
mp_bitcnt_t asc_ssv_lo(ssv_t const *x, int i);
mp_bitcnt_t asc_ssv_mod(mp_bitcnt_t j, ssv_t const *x, int i);
int asc_ssv_tstbit(ssv_t const *z, mp_bitcnt_t j);
struct fann_train_data *
fann_append_train_data(struct fann_train_data *, struct fann_train_data *);
struct fann *asc_update(struct fann *h, struct fann_train_data *D,
                        ssv_t const *z, ssv_t const *x, ssv_t const *e);
double asc_predict(ssv_t *u, ssv_t const *y, ssv_t const *e, struct fann *h);
int asc_ssv_combit(ssv_t *z, mp_bitcnt_t j);
int fann_save_safe(char const *path, struct fann *net);
ssv_t *asc_ssv_setbit(ssv_t *z, mp_bitcnt_t j);
int asc_ptrace_singlestep(pid_t pid, double period, int *status);
int asc_ptrace_baseline(pid_t pid);
double asc_online_predict(ssv_t *u, ssv_t const *y,
                          ssv_t const *e, struct fann *h);
