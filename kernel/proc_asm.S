#include <nros/macro.h>

	.global switch_proc
switch_proc:
	movl %cr3, %eax
	movl $UDATA_SEL, %eax 	
	pushl %eax 		# ss
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movl $0x111000, %eax
	pushl %eax		# esp
	pushfl
	popl %ebx
#	orl $0x200,%ebx
	pushl %ebx		# eflags
	movl $UCODE_SEL, %eax
	pushl %eax		# cs
	movl $0x110000, %eax
	pushl %eax		# eip
	iret