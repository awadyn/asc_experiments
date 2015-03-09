/* A small parser for Linux's /proc/pid/maps.  */

/* Reentrant parsing.  */
%parse-param {struct map **maps}
%parse-param {struct map *map}

%{
#include <asc.h>
#include <libgen.h>
#include <limits.h>
#include <sys/user.h>

extern FILE *yyin;
void yyerror(struct map **, struct map *, const char *);
int yyparse(struct map **, struct map *);
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
    map->name = strdup("[heap]");
    HASH_ADD_PTR(*maps, io.iov_base, map);
} | address mode offset dev inode pathname {
    HASH_ADD_PTR(*maps, io.iov_base, map);
}

address: lo dash hi;

lo: HEXADECIMAL {
    map = calloc(1, sizeof(struct map));
    map->io.iov_base = (void *)$1;
}

dash: CHAR;

hi: HEXADECIMAL {
    map->io.iov_len = (void *)$1 - map->io.iov_base;
}

mode: STRING {
    map->mode = 0;

    if ($1[0] == 'r')
        map->mode |= READ;

    if ($1[1] == 'w')
        map->mode |= WRITE;

    if ($1[2] == 'x')
        map->mode |= EXECUTE;
}

offset: HEXADECIMAL;

dev: HEXADECIMAL CHAR HEXADECIMAL;

inode: HEXADECIMAL;

pathname: STRING {
    map->name = strdup(basename($1));
}
%%

/* Called on parse errors.  */
void yyerror(struct map **maps, struct map *map, const char *s)
{
    fflush(stdout);
    fflush(stderr);
    diagnostic("parse error: %p: %p: %s", *maps, map, s);
    fflush(stderr);
}

/* Entry point.  */
struct map *parse(int pid, char const *file)
{
    struct map *map, *maps = 0;
    char path[PATH_MAX];
    FILE *f;

    if ((map = calloc(1, sizeof(struct map))) == 0) {
        diagnostic("calloc");
        goto failure;
    }

    map->io.iov_len = sizeof(struct user_regs_struct);
    map->mode = READ | WRITE;
    map->name = "[gprs]";

    HASH_ADD_PTR(maps, io.iov_base, map);

    if ((map = calloc(1, sizeof(struct map))) == 0) {
        diagnostic("calloc");
        goto failure;
    }

    map->io.iov_base = (void *) sizeof(struct user_regs_struct);
    map->io.iov_len = sizeof(struct user_fpregs_struct) + 320;
    map->mode = READ | WRITE;
    map->name = "[fprs]";

    HASH_ADD_PTR(maps, io.iov_base, map);

    /* Construct path to /proc/pid/maps file.  */
    if (file) {
        snprintf(path, sizeof(path) - 1, "%s", file);
    } else {
        if (pid == 0)
            sprintf(path, "/proc/self/maps");
        else
            sprintf(path, "/proc/%d/maps", pid);
    }

    /* Open the /proc/pid/maps file.  */
    if ((f = fopen(path, "r")) == 0) {
        diagnostic("fopen");
        goto failure;
    }

    /* Wire up lexer and parser to /proc/pid/maps file stream.  */
    yyin = f;
    
    /* Call lexer and parser until no input remains.  */
    do {
        if (yyparse(&maps, 0))
            goto failure;
    } while (feof(yyin) == 0);

    /* Release resources.  */
    fclose(f);

    return maps;

  failure:
    return 0;
}
