#pragma once

#define TTY_BUF_SIZE 1024

#define INC(a) ((a) = ((a)+1) & (TTY_BUF_SIZE-1))
#define DEC(a) ((a) = ((a)-1) & (TTY_BUF_SIZE-1))
#define EMPTY(a) ((a).head == (a).tail)
#define LEFT(a) (((a).tail-(a).head-1)&(TTY_BUF_SIZE-1))
#define LAST(a) ((a).buf[(TTY_BUF_SIZE-1)&((a).head-1)])
#define FULL(a) (!LEFT(a))
#define CHARS(a) (((a).head-(a).tail)&(TTY_BUF_SIZE-1))
#define GETCH(queue,c) \
  (void)({c=(queue).buf[(queue).tail];INC((queue).tail);})
#define PUTCH(c,queue) \
  (void)({(queue).buf[(queue).head]=(c);INC((queue).head);})

typedef struct tty_queue_s {
	unsigned long data;
	unsigned long head;
	unsigned long tail;
	char buf[TTY_BUF_SIZE];
} tty_queue_t;

typedef struct tty_s {
  tty_queue_t read_q;
  tty_queue_t write_q;
} tty_t;

void con_init();

/* 
 * XXX: need rewrite this and display.c
 * use this fucntion only in user mode via syscall,
 * in kernel mode use printf instead, or display
 * will mess up.
 */
void con_write(tty_t* tty);

extern tty_t tty;
