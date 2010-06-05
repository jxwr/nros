#include <nros/proc.h>
#include <nros/executable.h>
#include <nros/mm.h>
#include <nros/fs.h>
#include <nros/asm.h>
#include <stdio.h>
#include <list.h>
#include <string.h>

proc_t* current;
proc_t* idle_proc;
pid_t pid_idx;

link_t run_queue[MAX_PRIO + 1];

extern unsigned long kpagedir;

static void copy_kernel_vm(unsigned long* page_dir)
{ 
  /* 0x0c00 */
  int i;

  for(i = 0x300; i < 1024; i++) {
    page_dir[i] = (&kpagedir)[i];
  }
}

/* used for test */
void idle()
{
  asm volatile("1:\t"
	       "jmp 1b\n");
}

static void load_idle(proc_t* proc)
{
  unsigned long old_cr3;
  unsigned long cr3 = pa(proc->page_dir)|0x07;

  /* change address space temporarily */
  asm volatile("movl %%cr3, %0\n" 
	       "movl %1, %%ebx\n"
	       "movl %%ebx, %%cr3"
	       : "=r"(old_cr3)
	       : "m"(cr3));

  memcpy((void*)EXEC_TEXT_BASE, &idle, 0x100);

  /* back to the original address space */
  asm volatile("movl %0, %%cr3\n" :: "r"(old_cr3));
}


extern void start_proc();

void load_executable()
{
  int fd;

  fd = sys_open(current->filename);
  sys_read(fd, (void*)EXEC_TEXT_BASE, PAGE_SIZE);
  
  start_proc();
}

pid_t create_proc(char* name, char* filename, int prio)
{
  proc_t* proc = kmalloc(sizeof(proc_t));
  int i;

  proc->name = name;
  proc->filename = filename;
  proc->pid = pid_idx++;
  proc->prio = prio;
  proc->tick_left = TICK_QUANTUM;
  proc->page_dir = alloc_page_dir();


  copy_kernel_vm(proc->page_dir);
  proc->text_base = (void*)EXEC_TEXT_BASE;
  /* alloc one page */
  do_vm_alloc(proc, proc->text_base, PAGE_SIZE);

  proc->stack_base = (void*)EXEC_STACK_BASE;
  do_vm_alloc(proc, proc->stack_base - EXEC_INIT_STACK_SIZE, EXEC_INIT_STACK_SIZE);

  proc->hw_ctx.esp = (unsigned long)page_to_vir(alloc_pages(1)) + 0x2000;
  proc->hw_ctx.esp0 = proc->hw_ctx.esp;
  proc->hw_ctx.eip = (unsigned long)load_executable;

  for(i = 0; i < FILE_DESC_MAX; i++) {
    proc->filp[i] = NULL;
  }

  link_init(&proc->list);
  if(prio >= 0 && prio <= MAX_PRIO) {
    list_insert_after(&proc->list, &run_queue[prio]);
  }
  else {
    BUG_ON("create_proc");
  }
  
  return 0;
}

/*
 * destroy proc, and change address space to next;
 */
void destroy_proc(proc_t* proc)
{
  /* free all proc resource */
  do_vm_free(proc);
  free_pages(vir_to_page(proc->page_dir), 0);
  kfree(proc);
}

void proc_init()
{
  int i;

  pid_idx = 0;
  current = NULL;
  
  for(i = 0; i <= MAX_PRIO; i++) {
    link_init(&run_queue[i]);
  }

  create_proc("idle", "IDLE", PRIO_IDLE);
  idle_proc = current = link_to_struct(run_queue[PRIO_IDLE].next, proc_t, list);

  tss.esp0 = current->hw_ctx.esp;
  tss.ss0 = KDATA_SEL;

  create_proc("proc1", "TEST", PRIO_NORMAL);
}

void switch_to_user()
{
  unsigned long cr3;
  cr3 = pa(current->page_dir)|0x07;

  asm volatile("movl %0, %%esp\n\t"
	       "movl %1, %%cr3\n"
	       "jmp load_executable\n\t"
	       ::"m"(current->hw_ctx.esp),"r"(cr3));
}

int get_free_fd(proc_t* proc)
{
  int i;

  for(i = 0; i < FILE_DESC_MAX; i++) {
    if(proc->filp[i] == NULL)
      return i;
  }

  return -1;
}
