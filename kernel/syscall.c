#include <nros/common.h>
#include <nros/intr.h>
#include <nros/proc.h>
#include <nros/port.h>
#include <nros/syscall.h>
#include <stdio.h>
#include <string.h>



void do_sys_test()
{
  printf("%s\t", current->name);
}

void do_sys_idle()
{
  printf("idle ");
  asm volatile("sti\nhlt\n");
}

void do_sys_exit()
{
  proc_t* proc = current;

  printf("exit: %s\n", current->name);

  if(strcmp(current->name, "idle") == 0)
    BUG_MSG("idle can't exit.");

  list_remove(&current->list);
  /* free proc resource */
  destroy_proc(current);
  
  current = idle_proc;
  context_switch(proc, idle_proc);
}

void fastcall do_syscall(int sys_num)
{
  switch(sys_num) {
  case SYS_EXIT:
    do_sys_exit();
    break;
  case SYS_IDLE:
    do_sys_idle();
    break;
  default:
    printf("unknown syscall %d\n", sys_num);
    break;
  }
}
