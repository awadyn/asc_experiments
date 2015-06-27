k-CC = cc
k-CFLAGS = -D_BSD_SOURCE -D_GNU_SOURCE -nostdinc -I /usr/include/diet
k-CFLAGS += -Wall -Werror -Wno-attributes -Wno-deprecated-declarations
k-LDFLAGS = -g -nostdlib -static -L /usr/lib/diet/lib -lc -lm

%: kernels/%.s
	$(k-CC) $^ -Wall -W -Werror -g -nostdlib -static -o $@

%: kernels/crt0.s kernels/%.c
	$(k-CC) $^ $(k-CFLAGS) $(k-LDFLAGS) -o $@

ttt: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_STD_INPUT $(k-CFLAGS) $(k-LDFLAGS) -o $@

.PHONY: game1
.PHONY: game2
.PHONY: game3
.PHONY: game4
.PHONY: game5
.PHONY: game6
game1: tttgame1
game2: tttgame2
game3: tttgame3
game4: tttgame4
game5: tttgame5
game6: tttgame6
game7: tttgame7
game8: tttgame8
tttgame1: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"1013"' $(k-CFLAGS) $(k-LDFLAGS) -o $@

tttgame2: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"1513"' $(k-CFLAGS) $(k-LDFLAGS) -o $@

tttgame3: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"125076"' $(k-CFLAGS) $(k-LDFLAGS) -o $@

tttgame4: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"1247"' $(k-CFLAGS) $(k-LDFLAGS) -o $@

tttgame5: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"18624"' $(k-CFLAGS) $(k-LDFLAGS) -o $@

tttgame6: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"1124"' $(k-CFLAGS) $(k-LDFLAGS) -o $@

tttgame7: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"4671"' $(k-CFLAGS) $(k-LDFLAGS) -o $@

tttgame8: kernels/crt0.s kernels/ttt.c
	$(k-CC) $^ -D ASCIO_NO_OUTPUT -D ASCIO_INPUT='"60731"' $(k-CFLAGS) $(k-LDFLAGS) -o $@
