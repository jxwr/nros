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
extern void syscall();

extern void do_sys_test();
extern void do_sys_exit();
extern void do_syscall();

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

/* return if need reschedule */
int tick_advance()
{
  int need_resched = 0;
  
  current->tick_left--;
  if(current->tick_left < 0) {
    int prio = current->prio;

    need_resched = 1;
    current->tick_left = TICK_QUANTUM;
    list_remove(&current->list);
    list_insert_before(&run_queue[prio], current);
  }
  
  return need_resched;
}

void do_timer()
{
  if(tick_advance())
    schedule();
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
  idesc.flags = ACC_INTGATE | ACC_PRESENT;
  
  idesc.handler = &timer;
  set_idt_entry(0x20, &idesc);

  idesc.handler = &floppy;
  set_idt_entry(0x26, &idesc);

  idesc.handler = &keyboard_interrupt;
  set_idt_entry(0x21, &idesc);

  idesc.flags = ACC_INTGATE | ACC_PRESENT | ACC_DPL_RING3;
  idesc.handler = &syscall;
  set_idt_entry(0x40, &idesc);
}
