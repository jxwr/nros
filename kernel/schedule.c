#include <nros/common.h>
#include <nros/proc.h>
#include <stdio.h>

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
  proc_t* prev = current;
  proc_t* next = NULL;

  /*
  if(current->list.next != &proc_list)
    next = link_to_struct(current->list.next, proc_t, list);
  else
    next = link_to_struct(current->list.next->next, proc_t, list);
  current = next;
  */

  int i;
  for(i = MAX_PRIO; i >= 0; i--) {
    link_t* p = &run_queue[i];

    if(!list_empty(p)) {
      proc_t* proc = link_to_struct(p->next, proc_t, list);

      next = proc;
      current = next;

      context_switch(prev, next);
      break;
    }
  }
}
