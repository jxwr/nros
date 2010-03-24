include ../Makefile.inc

all: crt.o
	(cd libc; make)

crt.o: crt.S

clean:
	(cd libc; make clean)
	rm *.a *.o