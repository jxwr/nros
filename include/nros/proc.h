#pragma once
#include <list.h>
#include <nros/mm.h>
#include <nros/fs.h>

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
#define FILE_DESC_MAX 64

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
  file_t* filp[FILE_DESC_MAX];
} proc_t;

extern proc_t* current;
extern link_t proc_list;

void proc_init();

void switch_to_user();

pid_t create_proc();

/*
 * destroy proc, and change address space to next;
 */
void destroy_proc(proc_t* proc, proc_t* next);


/* schedule.c */
void schedule();

/* change address space to next, and switch to */
void context_switch(proc_t* prev, proc_t* next);

int get_free_fd(proc_t* proc);

#define switch_to(prev, next) do { \
  unsigned long esi, edi;	   \
  asm volatile("pushfl\n\t"	   \
	       "pushl %%ebp\n\t"   \
	       "movl %%esp,%0\n\t" \
	       "movl %4,%%esp\n\t" \
  	       "movl $1f,%1\n\t"   \
               "pushl %5\n\t"	   \
               "jmp __switch_to\n" \
               "1:\t"		   \
               "popl %%ebp\n\t"	   \
	       "popfl"		   \
  :"=m"(prev->hw_ctx.esp),"=m"(prev->hw_ctx.eip), \
   "=S"(esi),"=D"(edi)				  \
  :"m"(next->hw_ctx.esp),"m"(next->hw_ctx.eip),	\
	       "a"(prev),"d"(next));		  \
} while(0)
