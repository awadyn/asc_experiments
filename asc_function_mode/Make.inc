tools/libbmp:
	svn checkout http://libbmp.googlecode.com/svn/trunk/ tools/libbmp

tools/opt/lib/libbmp.a:  tools/libbmp
	cd tools/libbmp && ./configure --prefix=$(PWD)/tools/opt
	cd tools/libbmp && make install

2bmp: tools/opt/lib/libbmp.a tools/2bmp.c
	$(CC) ${CFLAGS} -O4  -Itools/opt/include -D __UI_STAND_ALONE__ tools/2bmp.c tools/opt/lib/libbmp.a -o 2bmp -lm

fann: tools/fann.o
	$(CC) $^ $(LDFLAGS) -o $@
