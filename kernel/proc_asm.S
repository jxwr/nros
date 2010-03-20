#include <nros/macro.h>

	.global switch_proc
switch_proc:
	popl %eax
	popl %eax
	movl %eax, %cr3
	
	movl $UDATA_SEL, %eax
	pushl %eax
	movl $4095, %eax
	pushl %eax
	pushfl
	popl %ebx
	orl $0x200,%ebx
	pushl %ebx
	movl $UCODE_SEL, %eax
	pushl %eax
	movl $0x2000, %eax
	pushl %eax
	iret