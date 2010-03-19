
/*
  the main gole of setup is to do some BIOS operations, and enter
  protected mode as soon as possbile. It use temp gdt and idt,
  we will then create real gdt and idt after we jump into the kernel.
*/
	
# these invariants are from OrzMicrokernel by Jserv
	
ACC_GRANULARY	=	0b1000000000000000
ACC_DEF32SIZE	=	0b0100000000000000
ACC_AVL		=	0b0001000000000000
ACC_LIMIT_MASK	=	0b0000111100000000
ACC_LIMIT_MUL	=	0b0000000100000000
ACC_PRESENT	=	0b10000000
ACC_DPL_RING3	=	0b01100000
ACC_DPL_RING2	=	0b01000000
ACC_DPL_RING1	=	0b00100000
ACC_DPL_RING0	=	0b00000000
ACC_USERSEG	=	0b00010000
ACC_SYSTEMSEG	=	0b00000000
ACC_TYPE_CODE	=	0b00001000
ACC_TYPE_DATA	=	0b00000000
ACC_TYPE_EXPDN	=	0b00000100	
ACC_TYPE_WRITE	=	0b00000010	
ACC_TYPE_CONF	=	0b00000100	
ACC_TYPE_READ	=	0b00000010	
ACC_ACCESSED	=	0b00000001

ACC_INTGATE	=	0b00001110
ACC_TRAPGATE	=	0b00001111
	
##################################
	
	.code16
	
	.section .setup.data, "a"
	.global gdtr, idtr
		       #-12345678901
krnl_name:	.ascii	"JXKERNEL   \n\r"
krnl_size:	
krnl_size_l:	.short	0
krnl_size_h:	.short	0
	
.align 0x10
gdtr:
gdt:		.short	gdt_end - gdt - 1
		.long	gdt
		.short	0
	
kernel_code:
		.short	0xffff
		.short	0x0000	
		.byte	0x00
		.short	ACC_GRANULARY | ACC_DEF32SIZE | \
			(0b1111 * ACC_LIMIT_MUL) | ACC_PRESENT | \
			ACC_DPL_RING0 | ACC_TYPE_CODE | \
			ACC_TYPE_READ | ACC_USERSEG
		.byte	0x00
kernel_data:
		.short	0xffff
		.short	0x0000	
		.byte	0x00
		.short	ACC_GRANULARY | ACC_DEF32SIZE | \
			(0b1111 * ACC_LIMIT_MUL) | ACC_PRESENT | \
			ACC_DPL_RING0 | ACC_TYPE_DATA | \
			ACC_TYPE_READ | ACC_USERSEG
		.byte	0x00
gdt_end:

idtr:		.short 	0x1000
		.long	0x0000
	

/*
 * 1. load kernel at 0x10000
 * 2. call main (remap pic, a20, dectect memory size, setup tables)
 * 3. long jump to pmode_entry from main, protected mode now
 * 4. goto kernel
 */	
	.section .setup.text, "ax"
	.global start, pmode_entry, die
start:	

	popw %ax		# load_file
	popw %dx		# putstr
	movw $0x1000,%bx
	movw %bx,%es
	xorw %bx,%bx
	movw %bx,%ds
	lea krnl_name,%si
	call *%dx		# putstr
	call *%ax		# load_file
	movw %si,krnl_size_l	# save kernel_size
	movw %di,krnl_size_h

	call main		# main does not return
	
	.code32
/*
 * reach here from main
 * just want to use 16-bit C to do sth.
 */
pmode_entry:
	movw $0x10,%ax
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs
	
	# move the kernel to 0x100000(1M).
	# not have to, just make it like a real kernel
	movw $0x10,%bx
	movw %bx,%ds
	movw %bx,%es
	movl $0x10000,%esi
	movl $0x100000,%edi

	movl krnl_size,%ecx
	rep movsl
dbg:
	# fly to kernel
	ljmpl $0x08,$0x100000

	# never got here
	.code16
die:	hlt
	jmp die
