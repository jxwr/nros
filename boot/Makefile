include ../Makefile.inc

../boot.bin: boot.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

boot.elf: boot.o
	$(LD) $(LDFLAGS) -T ./boot.ld $< -o $@

boot.o: boot.S

main.o: main.c
