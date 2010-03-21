#include <nros/proc.h>
#include <stdio.h>

void schedule()
{
  proc_t* next = NULL;
  if(cur_proc->list.next != &proc_list)
    next = link_to_struct(cur_proc->list.next, proc_t, list);
  else
    next = link_to_struct(cur_proc->list.next->next, proc_t, list);
  printf("proc name %s\n", next->name);
  cur_proc = next;
}
 
