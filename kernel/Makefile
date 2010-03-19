include ../Makefile.inc

CFLAGS += -I../include

OBJS = head.o main.o mm.o mm_alloc.o \
	intr.o intr_asm.o except.o except_asm.o \
	floppy.o disk_buffer.o fat12.o \
	keyboard.o keyboard_asm.o \
	mm_vm.o proc.o proc_asm.o

../kernel.bin: kernel.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

kernel.elf: $(OBJS) ../lib/libc.a
	$(LD) -T ./kernel.ld $(LDFLAGS) $^ -o $@

head.o: head.S

main.o: main.c

mm.o: mm.c ../include/nros/mm.h

mm_alloc.o: mm_alloc.c 	../include/nros/mm.h

mm_vm.o: mm_vm.c ../include/nros/mm.h

proc.o: proc.c

proc_asm.o: proc_asm.S

intr.o: intr.c

intr_asm.o: intr_asm.S

except.o: except.c

except_asm.o: except_asm.S

floppy.o: floppy.c

disk_buffer.o: disk_buffer.c

fat12.o: fat12.c

keyboard_asm.o: keyboard_asm.S

keyboard.o: keyboard.c

clean:
	rm *.o *.elf