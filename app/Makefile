include ../Makefile.inc

CFLAGS += -I../include
OBJS = test.o

../test.bin: test.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

test.elf: $(OBJS)
	$(LD) $(LDFLAGS) -T ./test.ld $^ -o $@

test.o: test.c

clean:
	rm *.o *.elf