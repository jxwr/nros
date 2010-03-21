include ../Makefile.inc

CFLAGS += -I../include
OBJS = setup.o main.o

../setup.bin: setup.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

setup.elf: $(OBJS)
	$(LD) $(LDFLAGS) -T ./setup.ld $^ -o $@

setup.o: setup.S

main.o: main.c

clean:
	rm *.o *.elf