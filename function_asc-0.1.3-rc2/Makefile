CPPFLAGS = -I .
CFLAGS = -ggdb3 -Wall -Wshadow -Wextra -Wno-unused-parameter \
         -Wno-unused-label -Wno-unused-but-set-variable -Werror
LDFLAGS = 
LDLIBS = -lfann -lcrypto -lgmp -ldl -lm

all: nops counter loop collatz factor 3sum asc

asc: main.o ptrace/start.o ssv/gather.o ssv/bind.o maps/lexer.o  \
     maps/parser.o ssv/calloc.o ssv/free.o peek/gpr.o poke/gpr.o \
     peek/fpr.o poke/fpr.o peek/region.o poke/region.o ssv/cmp.o \
     ssv/hash.o ssv/scan1.o ssv/swap.o ssv/pc.o ssv/qword.o      \
     ssv/scatter.o ssv/xor.o ssv/set.o ssv/popcount.o online/update.o \
     ssv/tstbit.o fann/append.o online/predict.o ssv/combit.o \
     online/save.o fann/save.o online/resume.o ssv/setbit.o \
     util/timestamp.o perf/attach.o perf/breakpoint.o \
     perf/instructions.o batch/train.o
	$(CC) $(strip $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS)) -o $@

main.o: display/default.i display/mispredict.i display/loop.i display/counter.i

%.o: %.c asc.h
	$(CC) $(strip $< $(CPPFLAGS) $(CFLAGS)) -fPIC -c -o $@

maps/lexer.c: maps/lexer.l maps/parser.h maps/parser.o
	$(LEX) -o $@ $<

maps/parser.h: maps/parser.o

maps/parser.c: maps/parser.y
	$(YACC) -o $@ --defines=maps/parser.h $<

k-CPPFLAGS = -nostdinc -isystem /usr/include/diet
k-CFLAGS = -ggdb3 -Wno-cpp
k-LDFLAGS = -static -nostdlib -L /usr/lib/diet/lib
k-LDLIBS = -lc -lm

%: kernels/crt0.s kernels/%.c
	$(CC) $(strip $^ $(k-CPPFLAGS) $(k-CFLAGS) $(k-LDFLAGS) $(k-LDLIBS)) -o $@

%: kernels/%.s
	$(CC) $^ -nostdinc -ggdb3 -nostdlib -static -o $@

CTAGS = ctags

tags:
	$(CTAGS) --recurse=yes --exclude=diet --exclude=numpy /usr/include $(CURDIR)

VALGRIND = valgrind
VFLAGS = -q --error-exitcode=1 --sim-hints=lax-ioctls --leak-check=full

clean:
	$(RM) $(wildcard *.a *.o */*.o *.so maps/parser.[ch] asc)

mrproper: clean
	$(RM) $(wildcard tags nops counter loop collatz factor)

.PHONY: all kernels emit test check clean mrproper verbose
