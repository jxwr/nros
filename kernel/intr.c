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

extern void sys_test();
extern void sys_exit();

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

unsigned long tick = 0;

void do_timer()
{
  tick++;
  if(tick % 1 == 0) {
    tick = 0;
    schedule();
  }
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

void do_sys_test()
{
  printf("%s\t", cur_proc->name);
}

void do_sys_exit()
{
  proc_t* prev = cur_proc;
  cur_proc = link_to_struct(cur_proc->list.next, proc_t, list);
  if(cur_proc == NULL)
    BUG_MSG("cur_proc is NULL");

  list_remove(&prev->list);
  /* free proc resource */
  destroy_proc(prev, cur_proc);
  switch_to(prev, cur_proc);
}

void intr_init()
{
  gate_desc_t idesc;

  mask_irq(IRQ_ENABLE_TIMER | IRQ_ENABLE_FLOPPY | IRQ_ENABLE_KEYBOARD);
  idesc.flags = ACC_INTGATE | ACC_PRESENT;
  
  idesc.handler = &timer;
  set_idt_entry(0x20, &idesc);

  idesc.handler = &floppy;
  set_idt_entry(0x26, &idesc);

  idesc.handler = &keyboard_interrupt;
  set_idt_entry(0x21, &idesc);

  idesc.flags = ACC_INTGATE | ACC_PRESENT | ACC_DPL_RING3;
  idesc.handler = &sys_test;
  set_idt_entry(0x40, &idesc);

  idesc.handler = &sys_exit;
  set_idt_entry(0x41, &idesc);
}
