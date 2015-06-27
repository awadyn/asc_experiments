#define _GNU_SOURCE

#include <errno.h>
#include <error.h>
#include <fann.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <uthash.h>
#include <linux/perf_event.h>
#include <sys/uio.h>

/* Runtime configuration flags.  */
enum flags {
    HELP         = 1 << 0,
};

/* Transition rule modes.  */
enum mode {
    BASELINE     = 0,
    SINGLESTEP   = 1 << 0,
    BLOCKSTEP    = 1 << 1,
    BREAKPOINT   = 1 << 2,
    INSTRUCTIONS = 1 << 3,
    FUNCTION = 1 << 4,
};

/* Memory map permissions.  */
enum permissions {
    EXECUTE      = 1 << 0,
    WRITE        = 1 << 1,
    READ         = 1 << 2,
};

/* Representation of a memory map.  */
struct map {
    struct iovec io;
    UT_hash_handle hh;
    unsigned int mode;
    char *name;
};


/* Representation of function return address linked list*/
struct rtn {
  long addr;
  struct rtn *next;
};


/* Representation of execution engine state.  */
struct machine {
    int flags;                     /* Runtime configuration flags.  */
    long rounds;                   /* Limit on number of rounds.  */
    char *metrics;                 /* Which telemetry to display.  */

    int mode;                      /* Transition rule mode.  */
    long address;                  /* Breakpoint address.  */
    long rtn_address;              /* Breakpoint address. */
    long period;                   /* Number of breakpoints per round.  */
    long instructions;             /* Number of instructions.  */

    int pid;                       /* Process id of child.  */
    struct map *maps;              /* Memory segments of child.  */
    struct iovec *io;              /* Scatter/gather I/O descriptors .  */
    long dim;                      /* State space dimensionality (bits).  */

    struct perf_event_attr pe;     /* Performance counter representations.  */
    int fd;                        /* Performance counter handles.  */

    struct rtn *rtn_stack;         /* List of return addresses of a function. */

    struct fann *model;
    mpz_t excitations;
    struct fann_train_data *data;

    double timestamp;              /* Wall clock time when machine started.  */
    double retired;                /* Number of instructions retired.  */

    long Bits;
};

/* Generic error message macro.  */
#define diagnostic(...)                                                      \
do {                                                                         \
    error_at_line(0, errno, __FILE__, __LINE__, __VA_ARGS__);                \
} while (0)

/* Generic trace macro.  */
#define tr()                                                                 \
do {                                                                         \
    fprintf(stderr, "%s:%d\n", __FUNCTION__, __LINE__);                      \
} while (0)

/* Generic print macro.  */
#define fprint(s, x)                                                         \
do {                                                                         \
    if (__builtin_types_compatible_p(typeof (x), long)) {                    \
        long _x = (long) x;                                                  \
        fprintf(s, "%ld\n", _x);                                             \
    } else if (__builtin_types_compatible_p(typeof (x), float)) {            \
        float _x = (float) x;                                                \
        fprintf(s, "%.10f\n", _x);                                           \
    } else {                                                                 \
        diagnostic("unknown type: " # x);                                    \
        __builtin_trap();                                                    \
    }                                                                        \
    fflush(s);                                                               \
} while (0)

/* Wrapper for generic print macro.  */
#define print(x) fprint(stdout, x)

/* Function signatures.  */
int options(struct machine *m, int argc, char *argv[]);
int help(void);
long initial(mpz_t y, mpz_t x, struct machine *m, int argc, char *argv[]);
int child(int argc, char *argv[]);
struct map *maps(int pid, char const *file);
int peek(mpz_t x, struct machine *m);
long integrate(mpz_t y, mpz_t const x, struct machine *m, enum mode o);
long transition(struct machine *m, enum mode o);
unsigned long sha(unsigned char *hash, unsigned char *raw, mpz_t const x);
double timestamp(struct timeval *tic);
int status(long r, mpz_t x, mpz_t y, mpz_t u, mpz_t d, mpz_t e,
           unsigned char *b, long a, struct machine *m);
int update(struct fann *h, struct fann_train_data *D,
           mpz_t const x, mpz_t const z, mpz_t const u, mpz_t const e);
int predict(mpz_t u, struct fann **h, struct fann_train_data *D,
            mpz_t const x, mpz_t const e);
void add(struct rtn **rtn_stack, long rtn_addr);
struct rtn* pop(struct rtn **rtn_stack);
