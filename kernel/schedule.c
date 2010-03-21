#include <nros/proc.h>
#include <stdio.h>

#define FASTCALL(x)     x __attribute__((regparm(3)))
#define fastcall          __attribute__((regparm(3)))

proc_t* fastcall __switch_to(proc_t* prev, proc_t* next)
{
  tss.esp0 = next->hw_ctx.esp0;
  return prev;
}

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

void context_switch(proc_t* prev, proc_t* next)
{
  unsigned long cr3 = pa(next->page_dir)|0x03;
  asm volatile("movl %0, %%ebx\n" 
	       "movl %%ebx, %%cr3":: "m"(cr3));
  printf("prev:%s\tnext:%s\n", prev->name, next->name);
  //  switch_to(prev, next);

do { \
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
 } while(0);
}

void schedule()
{
  proc_t* prev = cur_proc;
  proc_t* next = NULL;

  if(cur_proc->list.next != &proc_list)
    next = link_to_struct(cur_proc->list.next, proc_t, list);
  else
    next = link_to_struct(cur_proc->list.next->next, proc_t, list);
  cur_proc = next;
  
  context_switch(prev, next);
}
