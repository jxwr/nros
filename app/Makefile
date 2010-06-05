include ../Makefile.inc

CFLAGS += -I../include
OBJS = test.o idle.o

all: ../test.bin ../idle.bin

../test.bin: test.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

test.elf: test.o ../lib/crt.o
	$(LD) $(LDFLAGS) -T ./test.ld $^ -o $@

test.o: test.c

../idle.bin: idle.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

idle.elf: idle.o ../lib/crt.o
	$(LD) $(LDFLAGS) -T ./idle.ld $^ -o $@

idle.o: idle.c

clean:
	rm *.o *.elf