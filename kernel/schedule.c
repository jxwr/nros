#include <nros/proc.h>
#include <stdio.h>

#define FASTCALL(x)     x __attribute__((regparm(3)))
#define fastcall          __attribute__((regparm(3)))

proc_t* fastcall __switch_to(proc_t* prev, proc_t* next)
{
  tss.esp0 = next->hw_ctx.esp0;
  return prev;
}

void context_switch(proc_t* prev, proc_t* next)
{
  unsigned long cr3 = pa(next->page_dir)|0x03;
  asm volatile("movl %0, %%cr3":: "r"(cr3));
  switch_to(prev, next);
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
