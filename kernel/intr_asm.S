#include <nros/macro.h>
	
	.extern do_timer
	.global timer
	.global floppy
timer:
	pusha
	call do_timer
	movb $END_OF_INTERR,%al
	outb %al,$MASTER_PIC
	popa
	iret

floppy:
	pusha
	call do_floppy
	movb $END_OF_INTERR,%al
	outb %al,$MASTER_PIC
	popa
	iret