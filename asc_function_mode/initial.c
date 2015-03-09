#include <asc.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/unistd_64.h>
#include <sys/ptrace.h>
#include <sys/ioctl.h>
#include <sys/reg.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/wait.h>

long initial(mpz_t y, mpz_t x, struct machine *m, int argc, char *argv[])
{
    struct perf_event_attr *pe;
    int options, status;
    struct stat buf;
    struct map *map;
    long i;

    /* Record the wall clock time at which this machine started.  */
    m->timestamp = timestamp(0);

    /* Bail out if no kernel supplied.  */
    if (argc < 1) {
        if (write(2, "asc: error: no input kernel\n", 28) != 28)
            __builtin_trap();
        goto failure;
    }

    /* Bail out if kernel does not exist.  */
    if (stat(argv[0], &buf) < 0) {
        fprintf(stderr, "asc: `%s': %m\n", argv[0]);
        goto failure;
    }

    /* Start child process.  */
    switch (m->pid = fork()) {
    case 0:
        /* Replace process image with kernel.  */
        child(argc, argv);

        /* Bail out if error starting up process.  */
        exit(1);

        /* Should never be here.  */
        __builtin_trap();
    case -1:
        /* Bail out if error in starting child process.  */
        diagnostic("panic: fork failed");
        goto failure;
    default:
        break;
    }

    /* Bail out if child process create failed.  */
    if (waitpid(m->pid, &status, 0) == -1) {
        diagnostic("panic: waitpid on child process %d", m->pid);
        goto failure;
    }

    /* Bail out if child process create failed.  */
    if (WIFSTOPPED(status) == 0)
        goto failure;

    /* Ensure notification if child exits and no zombie processes.  */
    options = PTRACE_O_TRACEEXIT | PTRACE_O_EXITKILL;

    /* Install process tracing options.  */
    if (ptrace(PTRACE_SETOPTIONS, m->pid, 0, options) < 0) {
        diagnostic("ptrace setoptions");
        goto failure;
    }

    /* Get memory layout of child process.  */
    if ((m->maps = maps(m->pid, 0)) == 0)
        goto failure;

    /* Allocate scatter/gather I/O descriptors.  */
    if ((m->io = calloc(HASH_COUNT(m->maps), sizeof(struct iovec))) == 0) {
        diagnostic("calloc");
        goto failure;
    }

    /* Initialize scatter/gather I/O descriptors.  */
    for (map = m->maps, i = 0, m->dim = 0; map; map = map->hh.next, i++) {
        m->io[i] = map->io;
        m->dim += 8 * m->io[i].iov_len;
    }

    /* Get pointer to instruction count performance event structure.  */
    pe = &m->pe;

    /* Initialize instruction count performance event structure.  */
    pe->type = PERF_TYPE_HARDWARE;
#if 0
    pe->config = PERF_COUNT_SW_TASK_CLOCK;
#else
    pe->config = PERF_COUNT_HW_INSTRUCTIONS;
#endif
    pe->exclude_kernel = 1;
    pe->exclude_hv = 1;
    pe->exclude_idle = 1;
    pe->size = sizeof(struct perf_event_attr);

    if (m->mode & INSTRUCTIONS)
        pe->sample_period = m->instructions;

    if ((m->fd = syscall(__NR_perf_event_open, pe, m->pid, -1, -1, 0)) < 0) {
        diagnostic("perf_event_open");
        goto failure;
    }

    if (fcntl(m->fd, F_SETFL, O_ASYNC) < 0) {
        diagnostic("fcntl");
        goto failure;
    }

    if (fcntl(m->fd, F_SETSIG, SIGSTOP) < 0) {
        diagnostic("fcntl");
        goto failure;
    }

    if (fcntl(m->fd, F_SETOWN, m->pid) < 0) {
        diagnostic("fcntl");
        goto failure;
    }

    if (ioctl(m->fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
        diagnostic("ioctl enable");
        goto failure;
    }

    long rsp = ptrace(PTRACE_PEEKUSER, m->pid, 8 * RSP, 0);

    if (errno) {
        diagnostic("ptrace peek");
        goto failure;
    }

    for (i = rsp + sizeof(long) * (argc + 3); ; i += sizeof(Elf64_auxv_t)) {

        Elf64_auxv_t aux;

        aux.a_type = ptrace(PTRACE_PEEKDATA, m->pid, i, 0);

        if (errno) {
            diagnostic("ptrace peek data");
            goto failure;
        }

        aux.a_un.a_val = ptrace(PTRACE_PEEKDATA, m->pid, i, 0);

        if (errno) {
            diagnostic("ptrace peek data");
            goto failure;
        }

        if (aux.a_type == AT_NULL) {
            long qword;

            i += sizeof(Elf64_auxv_t);

            /* Scan past padding if any.  */
            do {

                qword = ptrace(PTRACE_PEEKDATA, m->pid, i, 0);

                if (errno) {
                    diagnostic("ptrace peek data");
                    goto failure;
                }

                i += sizeof(long);

            } while (qword == 0);

            if (ptrace(PTRACE_POKEDATA, m->pid, i - 1 * sizeof(long), 0) < 0) {
                diagnostic("ptrace poke");
                goto failure;
            }

            if (ptrace(PTRACE_POKEDATA, m->pid, i + 0 * sizeof(long), 0) < 0) {
                diagnostic("ptrace poke");
                goto failure;
            }

            if (ptrace(PTRACE_POKEDATA, m->pid, i + 1 * sizeof(long), 0) < 0) {
                diagnostic("ptrace poke");
                goto failure;
            }

            break;
        }
    }

    /* Fetch a representation of the initial state vector.  */
    if (peek(y, m) < 0)
        goto failure;

    /* Set "previous" state to the zero vector.  */
    mpz_xor(x, y, y);

    return m->dim;

  failure:
    return -1;
}
