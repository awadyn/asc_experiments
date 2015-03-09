VALGRIND = valgrind -q --error-exitcode=1 --sim-hints=lax-ioctls

.PHONY: check %.ok

check: help.ok singlestep.ok blockstep.ok breakpoint.ok instructions.ok

help.ok: asc
	$(VALGRIND) ./$< --help > /dev/null

singlestep.ok: asc loop
	$(VALGRIND) ./$< --mode=singlestep $(word 2, $^) > /dev/null

blockstep.ok: asc loop
	$(VALGRIND) ./$< --mode=blockstep $(word 2, $^) > /dev/null

breakpoint.ok: asc loop
	$(VALGRIND) ./$< --mode=breakpoint:400167 $(word 2, $^) > /dev/null

instructions.ok: asc loop
	$(VALGRIND) ./$< --mode=instructions:1.2e1 $(word 2, $^) > /dev/null

%: test/%.o
	$(CC) $^ $(LDFLAGS) -o $@

test/%.o: test/%.c include/asc.h
	$(CC) $< -I include $(CFLAGS) -c -o $@

tlb: maps/maps.o maps/parser.o maps/lexer.o

gather: maps/maps.o maps/parser.o maps/lexer.o
