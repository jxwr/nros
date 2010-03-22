#include <nros/common.h>
#include <nros/intr.h>
#include <nros/proc.h>
#include <nros/port.h>
#include <stdio.h>
#include <string.h>

/*
 * hardware interrupts handling and IDT operations
 */

extern char idt;
extern void timer();
extern void floppy();
extern void keyboard_interrupt();

typedef struct idt_entry_s {
  unsigned short handler_low16;
  unsigned short selector;
  unsigned short flags;
  unsigned short handler_high16;
} idt_entry_t;

void mask_irq(unsigned short mask)
{
  u8 v;
  u16 port;
  unsigned short _mask = ~mask;
  
  v = _mask >> 8;
  port = IRQ_HI_PORT;
  outb(v, port);
  
  v = _mask & 0x00ff;
  port = IRQ_LO_PORT;
  outb(v, port);
}

void set_idt_entry(int num, gate_desc_t* gdesc)
{
  char* idt_addr = &idt;
  idt_entry_t ie;

  ie.handler_low16 = (unsigned int)(gdesc->handler) & 0x0000ffff;
  ie.handler_high16 = (unsigned int)(gdesc->handler) >> 16;
  ie.selector = KCODE_SEL;
  ie.flags = (gdesc->flags | ACC_PRESENT) << 8;

  memcpy(idt_addr + num * sizeof(idt_entry_t), &ie, sizeof(idt_entry_t));
}

void do_timer()
{
  printf("TIMER START[\n");
  schedule();
  printf("]TIMER END\n");
}

extern unsigned long fd_irq_done;

void do_floppy()
{
  printf("floppy interrupt\n");
  fd_irq_done = 1;
}

void do_tty_interrupt()
{

}

void intr_init()
{
  gate_desc_t idesc;

  mask_irq(IRQ_ENABLE_TIMER | IRQ_ENABLE_FLOPPY | IRQ_ENABLE_KEYBOARD);
  idesc.flags = ACC_INTGATE;
  
  idesc.handler = &timer;
  set_idt_entry(0x20, &idesc);

  idesc.handler = &floppy;
  set_idt_entry(0x26, &idesc);

  idesc.handler = keyboard_interrupt;
  set_idt_entry(0x21, &idesc);

}
