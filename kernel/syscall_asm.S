#include <nros/macro.h>

	.global syscall
syscall:
	pusha
	call do_syscall
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
	