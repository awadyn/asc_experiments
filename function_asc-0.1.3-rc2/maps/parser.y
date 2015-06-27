/* A small parser for Linux's /proc/pid/maps.  */

/* Reentrant parsing.  */
%parse-param {int *M_r}
%parse-param {struct iovec *iov_r}
%parse-param {mode_t *mode_r}
%parse-param {char **name_r}

%{
#include <errno.h>
#include <error.h>
#include <gmp.h>
#include <fann.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/uio.h>
#include <linux/perf_event.h>
#include <asc.h>
#include <libgen.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

extern FILE *yyin;
void yyerror(int *, struct iovec *, mode_t *, char **, const char *);
int yyparse (int *, struct iovec *, mode_t *, char **);
int yylex();
%}

%union {
    char *string;
    unsigned long long hexadecimal;
    char character;
}

%token <string> STRING
%token <hexadecimal> HEXADECIMAL
%token <character> CHAR

%%
map: lines;

lines: lines line | line;

line: address mode offset dev inode {
} | address mode offset dev inode pathname {
}

address: lo dash hi;

lo: HEXADECIMAL {
#if 0
    printk("%llx", $1);
#endif
    iov_r->iov_base = (void *) $1;
}

dash: CHAR;

hi: HEXADECIMAL {
    iov_r->iov_len = (void *) $1 - iov_r->iov_base;
}

mode: STRING {
    *mode_r = 0;

    if ($1[0] == 'r')
        *mode_r |= S_IRUSR;

    if ($1[1] == 'w')
        *mode_r |= S_IWUSR;

    if ($1[2] == 'x')
        *mode_r |= S_IXUSR;
}

offset: HEXADECIMAL;

dev: HEXADECIMAL CHAR HEXADECIMAL;

inode: HEXADECIMAL;

pathname: STRING {
#if 0
    printk("%s", $1);
#endif

    if (name_r)
        if (*name_r)
            strncpy(*name_r, basename($1), NAME_MAX);

    /* Reentrant parsing.  */
    *M_r = *M_r + 1;
    iov_r++;
    mode_r++;

    if (name_r)
        name_r++;
}
%%

/* Called on parse errors.  */
void yyerror(int *M, struct iovec *iov, mode_t *mode,
             char **name, const char *str)
{
    fprintf(stderr, "%s:%d:%s: `%s' on record %d with address %p\n",
            __FILE__, __LINE__, __func__, str, *M, iov->iov_base);

    __builtin_trap();
}

/* Entry point.  */
int asc_maps_parse(struct iovec *iov, mode_t *mode, char **name,
                   int M, pid_t pid)
{
    char path[PATH_MAX];
    FILE *f;

    /* Construct path to /proc/pid/maps file.  */
    if (pid == 0)
        sprintf(path, "/proc/self/maps");
    else
        sprintf(path, "/proc/%d/maps", pid);

    /* Open the /proc/pid/maps file.  */
    if ((f = fopen(path, "r")) == 0) {
        printk("fopen `%s'", path);
        goto failure;
    }

    /* Wire up lexer and parser to /proc/pid/maps file stream.  */
    yyin = f;

    /* Reset memory maps count.  */
    M = 0;
    
    /* Call lexer and parser until no input remains.  */
    do {
        if (yyparse(&M, iov, mode, name))
            goto failure;
    } while (feof(yyin) == 0);

    /* Release resources.  */
    fclose(f);

    return M;

  failure:
    return 0;
}
