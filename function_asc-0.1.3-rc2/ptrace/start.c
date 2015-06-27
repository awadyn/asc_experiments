#define _GNU_SOURCE
#include <fann.h>
#include <linux/perf_event.h>
#include <errno.h>
#include <error.h>
#include <gmp.h>
#include <limits.h>
#include <sys/uio.h>
#include <stdint.h>
#include <stdlib.h>
#include <asc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/wait.h>

pid_t asc_ptrace_start(int argc, char *argv[])
{
    struct stat buf;
    long options;
    int status;
    pid_t pid;

    /* Bail out if no kernel supplied.  */
    if (argc < 1) {
        error(0, 0, "error: missing program argument");
        errno = EINVAL;
        return -errno;
    }

    /* Bail out if kernel does not exist.  */
    if (stat(argv[0], &buf) < 0) {
        printk("stat `%s'", argv[0]);
        goto failure;
    }

    /* Start child process.  */
    switch (pid = fork()) {
    case -1:
        /* Bail out if error in starting child process.  */
        error(1, errno, "panic: fork failed");
        goto failure;
    case 0:
        /* Prepend dot slash if necessary.  */
        if (strchr(argv[0], '/') == 0)
            if (asprintf(&argv[0], "./%s", argv[0]) < 0)
                error(1, errno, "asprintf");

        /* Make sure arguments are delimited by a null pointer.  */
        if (argv[argc] != 0)
            error(1, 0, "argv must be zero terminated");

        /* Zero out all environment variables.  */
        if (clearenv() < 0)
            error(1, errno, "clearenv");

        /* Disable address space layout randomization.  */
        if (personality(ADDR_NO_RANDOMIZE))
            error(1, errno, "child pid `%d' panic: personality", getpid());

        /* Volunteer control over child execution to parent.  */
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            char path[] = "/proc/sys/kernel/yama/ptrace_scope";
            FILE *stream;
            int scope;

            if ((stream = fopen(path, "r")) == 0) {
                printk("fopen `%s'", path);
                exit(1);
            }

            if (fscanf(stream, "%d", &scope) != 1)
                error(1, errno, "fscanf `%s'", path);

            if (scope > 1)
                error(1, 0, "please enable ptrace in `%s'", path);

            error(1, errno, "ptrace TRACEME");
        }

        /* Replace process image with kernel.  */
        execvpe(argv[0], argv, 0);

        /* We should not be here.  */
        error(1, errno, "execvp `%s'", argv[0]);

        /* Bail out if error starting up process.  */
        exit(1);

        /* Should never be here.  */
        __builtin_trap();
    default:
        break;
    }

    /* Bail out if child process create failed.  */
    if (waitpid(pid, &status, 0) == -1) {
        printk("waitpid on child pid `%d'", pid);
        goto failure;
    }

    /* Bail out if child process is not stopped.  */
    if (WIFSTOPPED(status) == 0)
        goto failure;

    /* Ensure notification if child exits and no zombie processes.  */
    options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT | PTRACE_O_EXITKILL;

    /* Install process tracing options.  */
    if (ptrace(PTRACE_SETOPTIONS, pid, 0, options) < 0) {
        printk("ptrace SETOPTIONS");
        goto failure;
    }

    /* An attempt to make SHA-256 hashes repeatable.  */
    if (0) {
        uint64_t fixup[] = {
            0x7fffffffefc9,
            0x7fffffffefca,
            0x7fffffffefcb,
            0x7fffffffefcc,
            0x7fffffffefcd,
            0x7fffffffefce,
            0x7fffffffefcf,
            0x7fffffffefd0,
            0x7fffffffefd1,
            0x7fffffffefd2,
            0x7fffffffefd3,
            0x7fffffffefd4,
            0x7fffffffefd5,
            0x7fffffffefd6,
            0x7fffffffefd7,
            0x7fffffffefd8,
            0x0,
        };

        int i;

        for (i = 0; fixup[i] != 0; i++) {
            void *addr = (void *)(fixup[i] & ~0x3);

            uint64_t qword = ptrace(PTRACE_PEEKDATA, pid, addr, 0);

            if (errno)
                return -errno;

            // printk("%016lx", qword);
            qword = qword & ~(0xff << ((fixup[i] - (uint64_t) addr) * 8));
            // printk("%016lx\n", qword);

            if (ptrace(PTRACE_POKEDATA, pid, addr, qword) < 0) {
                printk("%s: ptrace POKEDATA", __func__);
                goto failure;
            }
        }
    }

    return pid;

  failure:
    return -1;
}
