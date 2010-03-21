#include <nros/macro.h>

	.code32
	.global start
	.section .head.text, "ax"
	
start:
		# timer set to 100Hz
		movb $0x34,%al
		outb %al,$0x43
		movb $0x9b,%al
		outb %al,$0x40
		movb $0x2e,%al
		outb %al,$0x40

		movl $tss, %eax			# set tss . I don't know how to use
		movw %ax,asm_pa(tss_seg+2)	# static method to set tss struct values.
		shrl $16,%eax			# address is 32 bits, and I want low 16
		movb %al,asm_pa(tss_seg+4)	# bits of it, but can't use '&' with address
		movb %ah,asm_pa(tss_seg+7)

		# do the kernel space mapping (4M)
		movl $asm_pa(kpagetab)+0x07,asm_pa(kpagedir)		# 1:1 mapping
		movl $asm_pa(kpagetab)+0x07,asm_pa(kpagedir)+0x0c00	# map 0xc0000000 to 0
		movl $asm_pa(kpagedir)+0x07,%edx
		movl %edx,%cr3

		# fill page table
		movl $asm_pa(kpagetab),%edi
		movl $0x03,%eax
		movl $1024,%ecx
start0:		stosl
		addl $0x1000,%eax
		loop start0

		# enable paging
		movl %cr0,%eax
		orl $0x80000000,%eax
		movl %eax,%cr0

		movl $KVM_STACK,%ebx
		movl %ebx,%ebp
		movl %ebx,%esp
	
		lgdtl asm_pa(gdtr)
		lidtl asm_pa(idtr)
		
		movw $TSS_SEL,%ax
		ltr  %ax

		ljmpl $0x08,$real_start

real_start:	movw $KDATA_SEL,%bx
		movw %bx,%ds
		movw %bx,%es
		movw %bx,%fs
		movw %bx,%ss

	.global kpagedir,gdt,idt,tss
		call clear_screen
		call main
	
die:		hlt
		jmp die

##################################################################
	
	.section .head.data, "a"
Msg:		.asciz	"kernel   "
curPos:		.short	0x0000
color:		.byte	0x07
	
	.section .head.table, "a"
kpagedir:	.fill 1024,4,0
kpagetab:	.fill 1024,4,0
	
	.align 0x10
gdtr:
gdt:		.short	gdt_end - gdt - 1
		.long 	gdt
		.short 	0

		# limit 00..15
		# base 00..15
		# base 16..23   /-- limit 16..19
		# G D 0 AVL LIMIT(4) P DPL(2) S TYPE(3) A
		# base 24..31

kernel_code_seg:
		.short 	0xFFFF
		.short 	0x0000
		.byte 	0x00
		.short 	ACC_GRANULARY | ACC_DEF32SIZE | \
			(0b1111 * ACC_LIMIT_MUL) | ACC_PRESENT | \
			ACC_DPL_RING0 | ACC_TYPE_CODE | \
			ACC_TYPE_READ | ACC_USERSEG
		.byte 	0x00

kernel_data_seg:
		.short 	0xFFFF
		.short 	0x0000
		.byte 	0x00
		.short 	ACC_GRANULARY | ACC_DEF32SIZE | \
			(0b1111 * ACC_LIMIT_MUL) | ACC_PRESENT | \
			ACC_DPL_RING0 | ACC_TYPE_DATA | \
			ACC_TYPE_WRITE | ACC_USERSEG
		.byte 	0x00	
	
user_code_seg:
		.short 	0xFFFF
		.short 	0x0000
		.byte 	0x00
		.short 	ACC_GRANULARY | ACC_DEF32SIZE | \
			(0b1111 * ACC_LIMIT_MUL) | ACC_PRESENT | \
			ACC_DPL_RING3 | ACC_TYPE_CODE | \
			ACC_TYPE_READ | ACC_USERSEG
		.byte 	0x00

user_data_seg:
		.short 	0xFFFF
		.short 	0x0000
		.byte 	0x00
		.short 	ACC_GRANULARY | ACC_DEF32SIZE | \
			(0b1111 * ACC_LIMIT_MUL) | ACC_PRESENT | \
			ACC_DPL_RING3 | ACC_TYPE_DATA | \
			ACC_TYPE_WRITE | ACC_USERSEG
		.byte 	0x00
	
tss_seg:
		.short 	0x0067		#Limit (Lower 16 bits)
		.short 	0x0000		#Base (Lower 16 bits)
		.byte 	0x00		#Base (Mi.longle 8 bits)
		.byte 	0x89		#Present, Ring 3, 32-bit TSS
		.byte 	0x00		#Granularity, 0, 0, AVL, Limit
		.byte 	KVM_BASE >> 24	#Base
gdt_end:
	
	.align	0x10
tss:		.fill	0x67,1,0
tss_end:
	
	.align	0x10
idtr:		.short 	idt_end - idt - 1
		.long	idt

	.align	0x10
idt:		.fill	256,8,0
idt_end:

