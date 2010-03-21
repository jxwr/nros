#pragma once

#define	ACC_INTGATE	0b00001110 	// _IDT
#define ACC_TRAPGATE	0b00001111 	// _ID

typedef struct gate_desc_s {
  unsigned short flags;
  void (*handler)();
} gate_desc_t;



void intr_init();
void except_init();

void set_idt_entry(int num, gate_desc_t* idesc);


#define	IRQ_ENABLE_TIMER	0b0000000000000001
#define	IRQ_ENABLE_KEYBOARD     0b0000000000000010
#define IRQ_ENABLE_FLOPPY       0b0000000001000000

void mask_irq(unsigned short mask);
