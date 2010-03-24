
.section .text
	.global start
start:
	pushl $crt_exit
	jmp main


crt_exit:	
	int $0x41