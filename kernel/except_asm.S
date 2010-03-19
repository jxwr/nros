
.global	divide_error, 	debug_exception
.global	nmi_interrupt, 	breakpoint
.global	bound_check,	coprocessor_not_available
.global	double_fault, 	coprocessor_segment_overrun
.global	invalid_tss,	segment_not_present
.global	stack_exception,general_protection
.global	page_fault,	invalid_opcode
.global	overflow,	coprocessor_error
.global	machine_check, 	alignment_check
.global	intel_reserved,	simd_floating_point
		
	
divide_error:
		pusha
		cli
		call do_divide_error
		popa
		iret
	
debug_exception:	
		pusha
		cli
		call do_debug_exception
		popa
		iret
	
nmi_interrupt:	
		pusha
		cli
		call do_nmi_interrupt
		popa
		iret
	
breakpoint:	
		pusha
		cli
		call do_breakpoint
		popa
		iret

overflow:	
		pusha
		cli
		call do_overflow
		popa
		iret
	
bound_check:	
		pusha
		cli
		call do_bound_check
		popa
		iret
	
invalid_opcode:	
		pusha
		cli
		call do_invalid_opcode
		popa
		iret
	
coprocessor_not_available:	
		pusha
		cli
		call do_coprocessor_not_available
		popa
		iret
	
double_fault:	
		pusha
		cli
		call do_double_fault
		popa
		iret
		
coprocessor_segment_overrun:	
		pusha
		cli
		call do_coprocessor_segment_overrun
		popa
		iret
	
invalid_tss:	
		pusha
		cli
		call do_invalid_tss
		popa
		iret
	
segment_not_present:	
		pusha
		cli
		call do_segment_not_present
		popa
		iret
	
stack_exception:	
		pusha
		cli
		call do_stack_exception
		popa
		iret
	
general_protection:	
		pusha
		cli
		call do_general_protection
		popa
		iret
	
page_fault:	
		pusha
		cli
		call do_page_fault
		popa
		iret

intel_reserved:	
		pusha
		cli
		call do_intel_reserved
		popa
		iret
	
coprocessor_error:	
		pusha
		cli
		call do_coprocessor_error
		popa
		iret
	
alignment_check:	
		pusha
		cli
		call do_alignment_check
		popa
		iret
	
machine_check:	
		pusha
		cli
		call do_machine_check
		popa
		iret
	
simd_floating_point:	
		pusha
		cli
		call do_simd_floating_point
		popa
		iret