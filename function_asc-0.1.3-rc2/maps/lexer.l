/* A small lexical analyzer for parsing Linux's /proc/pid/maps.  */

%option noyywrap
%option nounput

%{
#include <errno.h>
#include <error.h>
#include <fann.h>
#include <gmp.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/uio.h>
#include <linux/perf_event.h>
#include <asc.h>

#include <stdio.h>
#include <string.h>

#define YY_DECL extern int yylex()
#define YY_NO_INPUT

#include "parser.h"
%}

%%
[a-f0-9]+ {
    errno = 0;
    yylval.hexadecimal = strtoull(yytext, 0, 16); 
    if (errno)
        error(0, errno, "strtoull");
    return HEXADECIMAL; 
}

[\-\:] {
    yylval.character = yytext[0];
    return CHAR; 
}

[rwxp-]{4} {
    yylval.string = yytext;
    return STRING;
}

[\/\[]{1}[a-zA-Z0-9\]\.\_\-\/\:]+ {
    yylval.string = yytext;
    return STRING; 
}

[ \n] {
    ; 
}
%%
