include ../Makefile.inc

CFLAGS += -I../include

OBJS = head.o main.o mm.o mm_alloc.o \
	intr.o intr_asm.o except.o except_asm.o \
	floppy.o disk_buffer.o fat12.o \
	keyboard.o keyboard_asm.o \
	mm_vm.o proc.o proc_asm.o \
	schedule.o console.o fs.o \

HEADERS = ../include/nros/mm.h ../include/nros/proc.h ../include/nros/fs.h

../kernel.bin: kernel.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

kernel.elf: $(OBJS) ../lib/libc.a
	$(LD) -T ./kernel.ld $(LDFLAGS) $^ -o $@

head.o: head.S

main.o: main.c mm_alloc.c $(HEADERS)

mm.o: mm.c mm_alloc.c $(HEADERS)

mm_alloc.o: mm_alloc.c $(HEADERS)

mm_vm.o: mm_vm.c $(HEADERS)

proc.o: proc.c $(HEADERS)

proc_asm.o: proc_asm.S

schedule.o: schedule.c $(HEADERS)

intr.o: intr.c $(HEADERS)

intr_asm.o: intr_asm.S

except.o: except.c $(HEADERS)

except_asm.o: except_asm.S $(HEADERS)

floppy.o: floppy.c $(HEADERS)

disk_buffer.o: disk_buffer.c $(HEADERS)

fat12.o: fat12.c $(HEADERS)

keyboard_asm.o: keyboard_asm.S

keyboard.o: keyboard.c $(HEADERS)

console.o: console.c

fs.o: fs.c

clean:
	rm *.o *.elf