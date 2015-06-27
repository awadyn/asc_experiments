CC = cc
CFLAGS = -g3 -ggdb
LDFLAGS = -g3 -ggdb -lfann -lgmp -lcrypto -lm
CLEAN = asc train *.[ch]~ *.o */*.[ch]~ */*.o
MRPROPER = nops counter loop collatz rsa inner 3sum *.excite *.train *.net

ifndef W
  CFLAGS += -Wall -Wextra -Wshadow -Werror
  CFLAGS += -Wno-unused-parameter -Wno-unused-variable
  LDFLAGS += -Wall -Werror
endif

ifdef O
  CFLAGS += -O$(O)
  LDFLAGS += -O$(O)
else
  CFLAGS += -fno-omit-frame-pointer -fsanitize=address
  LDFLAGS += -fno-omit-frame-pointer -fsanitize=address
endif

asc: main.o options.o help.o initial.o child.o peek.o integrate.o transition.o \
     status.o sha.o timestamp.o predict.o update.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c include/asc.h
	$(CC) $< -I include $(CFLAGS) -c -o $@

include maps/Make.inc
include test/Make.inc
include kernels/Make.inc

.PHONY: all clean mrproper

clean:
	$(RM) $(wildcard $(CLEAN))

mrproper:
	$(RM) $(wildcard $(CLEAN) $(MRPROPER))
