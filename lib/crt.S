
.section .text
	.global start
start:
	pushl $crt_exit
	jmp main


crt_exit:
	movl $0x01, %eax
	int $0x40