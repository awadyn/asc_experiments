#include <asc.h>
#include <unistd.h>
#include <sys/personality.h>
#include <sys/ptrace.h>

int child(int argc, char *argv[])
{
    /* Prepend dot slash if necessary.  */
    if (strchr(argv[0], '/') == 0) {
        if (asprintf(&argv[0], "./%s", argv[0]) < 0) {
            diagnostic("asprintf");
            goto failure;
        }
    }

    /* Make sure arguments are delimited by a null pointer.  */
    if (argv[argc] != 0) {
        diagnostic("argv must be zero terminated");
        goto failure;
    }

    /* Zero out all environment variables.  */
    if (clearenv() < 0) {
        diagnostic("clearenv");
        goto failure;
    }

    /* Disable address space layout randomization.  */
    if (personality(ADDR_NO_RANDOMIZE)) {
        diagnostic("personality");
        goto failure;
    }

    /* Volunteer control over child execution to parent.  */
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        diagnostic("ptrace traceme");
        goto failure;
    }

    /* Replace process image with kernel.  */
    execvpe(argv[0], argv, 0);

    /* We should not be here.  */
    diagnostic("execvp: %s", argv[0]);

  failure:
    return 1;
}
