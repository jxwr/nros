#include <nros/macro.h>
	
	.global timer
timer:
	pusha
	cli
	call do_timer
	movb $END_OF_INTERR,%al
	outb %al,$MASTER_PIC
	popa
	iret
	
	.global floppy
floppy:
	pusha
	cli
	call do_floppy
	movb $END_OF_INTERR,%al
	outb %al,$MASTER_PIC
	popa
	iret

	.global sys_test
sys_test:
	pusha
	call do_sys_test
	popa
	iret

	.global sys_exit
sys_exit:
	pusha
	call do_sys_exit
	popa
	iret