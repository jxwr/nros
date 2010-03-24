#include <nros/macro.h>
#include <nros/executable.h>

	.global start_proc
start_proc:
	movl $UDATA_SEL, %eax 	
	pushl %eax 		# ss
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movl $EXEC_STACK_BASE, %eax
	pushl %eax		# esp
	pushfl
	popl %ebx
	orl $0x200,%ebx
	pushl %ebx		# eflags
	movl $UCODE_SEL, %eax
	pushl %eax		# cs
	movl $EXEC_TEXT_BASE, %eax
	pushl %eax		# eip

	/*
	 * a new created process starts when scheduled, and schedule
	 * happens in interrupts or syscalls, when interrupts happen,
	 * we should send END_OF_INTERR to PIC after handling the
	 * interrupt, now we just send it in both situations
	 */
	movb $END_OF_INTERR,%al
	outb %al,$MASTER_PIC	
	iret
