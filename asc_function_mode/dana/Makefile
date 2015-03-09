//CFLAGS=-g 
CFLAGS=-O4
 
danasrv: uio_helper.o danasrv.o if_helper.o
	$(CC) -o danasrv uio_helper.o if_helper.o danasrv.o $(CFLAGS)

uio_helper.o: uio_helper.h uio_helper.c
	$(CC) -c uio_helper.c $(CFLAGS)

if_helper.o: if_helper.h if_helper.c
	$(CC) -c if_helper.c $(CFLAGS)

danasrv.o: uio_helper.h danasrv.c
	$(CC) -c danasrv.c $(CFLAGS)

clean:
	$(RM) $(wildcard *.o *~ danasrv)
