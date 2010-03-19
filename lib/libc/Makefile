include ../../Makefile.inc

OBJS = string.o display.o

CFLAGS += -I../../include

../libc.a: $(OBJS)
	ar crv $@ $^

string.o: string.c ../../include/string.h

display.o: display.c ../../include/stdio.h


clean:
	rm *.o