#include <nros/common.h>
#include <nros/intr.h>

void divide_error();
void debug_exception();
void nmi_interrupt();
void breakpoint();
void overflow();
void bound_check();
void invalid_opcode();
void coprocessor_not_available();
void double_fault();
void coprocessor_segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_exception();
void general_protection();
void page_fault();
void intel_reserved();
void coprocessor_error();
void alignment_check();
void machine_check();
void simd_floating_point();

void do_divide_error()
{
  panic("except 0, divide error.");
}

void do_debug_exception()
{
  panic("except 1, debug exception.");
}

void do_nmi_interrupt()
{
  panic("except 2, nmi interrupt.");
}

void do_breakpoint()
{
  panic("except 3, breakpoint.");
}

void do_overflow()
{
  panic("except 4, overflow.");
}

void do_bound_check()
{
  panic("except 5, bound_check.");
}

void do_invalid_opcode()
{
  panic("except 6, invalid opcode.");
}

void do_coprocessor_not_available()
{
  panic("except 7, coprocessor not available.");
}

void do_double_fault()
{
  panic("except 8, double fault.");
}

void do_coprocessor_segment_overrun()
{
  panic("except 9, coprocessor segment overrun.");
}

void do_invalid_tss()
{
  panic("except 10, invalid tss.");
}

void do_segment_not_present()
{
  panic("except 11, segment not present.");
}

void do_stack_exception()
{
  panic("except 12, stack exception.");
}

void do_general_protection()
{
  panic("except 13, general protection.");
}

void do_page_fault()
{
  panic("except 14, page fault.");
}

void do_intel_reserved()
{
  panic("intel reserved.");
}

void do_coprocessor_error()
{
  panic("except 16, coprocessor error.");
}

void do_alignment_check()
{
  panic("except 17, alignment check.");
}

void do_machine_check()
{
  panic("except 18, machine check.");
}

void do_simd_floating_point()
{
  panic("except 19, SIMD Floating-Point.");
}

void except_init()
{
  gate_desc_t gdesc;

  gdesc.flags = ACC_TRAPGATE;

  gdesc.handler = &divide_error;
  set_idt_entry(0, &gdesc);
  gdesc.handler = &debug_exception;
  set_idt_entry(1, &gdesc);
  gdesc.handler = &nmi_interrupt;
  set_idt_entry(2, &gdesc);
  gdesc.handler = &breakpoint;
  set_idt_entry(3, &gdesc);
  gdesc.handler = &overflow;
  set_idt_entry(4, &gdesc);
  gdesc.handler = &bound_check;
  set_idt_entry(5, &gdesc);
  gdesc.handler = &invalid_opcode;
  set_idt_entry(6, &gdesc);
  gdesc.handler = &coprocessor_not_available;
  set_idt_entry(7, &gdesc);
  gdesc.handler = &double_fault;
  set_idt_entry(8, &gdesc);
  gdesc.handler = &coprocessor_segment_overrun;
  set_idt_entry(9, &gdesc);
  gdesc.handler = &invalid_tss;
  set_idt_entry(10, &gdesc);
  gdesc.handler = &segment_not_present;
  set_idt_entry(11, &gdesc);
  gdesc.handler = &stack_exception;
  set_idt_entry(12, &gdesc);
  gdesc.handler = &general_protection;
  set_idt_entry(13, &gdesc);
  gdesc.handler = &page_fault;
  set_idt_entry(14, &gdesc);
  gdesc.handler = &intel_reserved;
  set_idt_entry(15, &gdesc);
  gdesc.handler = &coprocessor_error;
  set_idt_entry(16, &gdesc);
  gdesc.handler = &alignment_check;
  set_idt_entry(17, &gdesc);
  gdesc.handler = &machine_check;
  set_idt_entry(18, &gdesc);
  gdesc.handler = &simd_floating_point;
  set_idt_entry(19, &gdesc);
}
