CLEAN += maps/*.o maps/lexer.c maps/parser.[ch]

asc: maps/maps.o maps/parser.o maps/lexer.o

maps/lexer.c: maps/lexer.l maps/parser.h maps/parser.o
	$(LEX) -o $@ $<

maps/parser.h: maps/parser.o

maps/parser.c: maps/parser.y
	$(YACC) -o $@ --defines=maps/parser.h $<
