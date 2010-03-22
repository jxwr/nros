#pragma once
#include <list.h>
#include <nros/mm.h>

typedef struct tss_s {
  long	back_link;	/* 16 high bits zero */
  long	esp0;
  long	ss0;		/* 16 high bits zero */
  long	esp1;
  long	ss1;		/* 16 high bits zero */
  long	esp2;
  long	ss2;		/* 16 high bits zero */
  long	cr3;
  long	eip;
  long	eflags;
  long	eax,ecx,edx,ebx;
  long	esp;
  long	ebp;
  long	esi;
  long	edi;
  long	es;		/* 16 high bits zero */
  long	cs;		/* 16 high bits zero */
  long	ss;		/* 16 high bits zero */
  long	ds;		/* 16 high bits zero */
  long	fs;		/* 16 high bits zero */
  long	gs;		/* 16 high bits zero */
  long	ldt;		/* 16 high bits zero */
  long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
} tss_t;

extern tss_t tss;
/* hardware context */
typedef struct hw_ctx_s {
  unsigned long esp0;
  unsigned long esp;
  unsigned long eip;
} hw_ctx_t;

typedef unsigned int pid_t;

/* process struct */
typedef struct proc_s {
  pid_t pid;
  vm_info_t vm_info;
  void* text_base;
  void* stack_base;
  void* page_dir; /* x86, page directory virtual address */
  link_t list;
  char* name;
  hw_ctx_t hw_ctx;
} proc_t;

extern proc_t* cur_proc;
extern link_t proc_list;


void proc_init();

pid_t create_proc();

void schedule();

