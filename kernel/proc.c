#include <nros/proc.h>
#include <nros/executable.h>
#include <nros/mm.h>
#include <nros/fat12.h>
#include <nros/asm.h>
#include <stdio.h>
#include <list.h>
#include <string.h>

link_t proc_list;
proc_t* cur_proc;
pid_t pid_idx;

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

void load_executable(proc_t* proc, char* filename)
{
  unsigned long old_cr3;
  unsigned long cr3 = pa(proc->page_dir)|0x07;

  /* change address space temporarily */
  asm volatile("movl %%cr3, %0\n" 
	       "movl %1, %%ebx\n"
	       "movl %%ebx, %%cr3"
	       : "=r"(old_cr3)
	       : "m"(cr3));

  fat12_read(filename, 0, (void*)EXEC_TEXT_BASE, PAGE_SIZE);

  /* back to the original address space */
  asm volatile("movl %0, %%cr3\n" :: "r"(old_cr3));
}

extern void start_proc();


pid_t create_proc(char* name, char* filename)
{
  proc_t* proc = kmalloc(sizeof(proc_t));
  proc->name = name;
  proc->pid = pid_idx++;
  proc->page_dir = alloc_page_dir();

  copy_kernel_vm(proc->page_dir);
  proc->text_base = (void*)EXEC_TEXT_BASE;
  /* alloc one page */
  do_vm_alloc(proc, proc->text_base, PAGE_SIZE);

  proc->stack_base = (void*)EXEC_STACK_BASE;
  do_vm_alloc(proc, proc->stack_base - EXEC_INIT_STACK_SIZE, EXEC_INIT_STACK_SIZE);

  proc->hw_ctx.esp = (unsigned long)page_to_vir(alloc_pages(1)) + 0x2000;
  proc->hw_ctx.esp0 = proc->hw_ctx.esp;
  proc->hw_ctx.eip = (unsigned long)start_proc;

  load_executable(proc, filename);

  link_init(&proc->list);
  list_insert_after(&proc->list, &proc_list);
  if(cur_proc == NULL) {
    load_idle(proc);
    cur_proc = proc;
  }

  return 0;
}

/*
 * destroy proc, and change address space to next;
 */
void destroy_proc(proc_t* proc, proc_t* next)
{
  unsigned long cr3;

  /* switch to 'next address space*/
  cr3 = pa(next->page_dir)|0x07;
  asm volatile("movl %0, %%cr3\n" :: "r"(cr3));

  /* free all proc resource */
  do_vm_free(proc);
  free_pages(vir_to_page(proc->page_dir), 0);
  kfree(proc);
}

void proc_init()
{
  unsigned long cr3;

  pid_idx = 0;
  cur_proc = NULL;
  link_init(&proc_list);
  create_proc("idle", "TEST");
  tss.esp0 = cur_proc->hw_ctx.esp;
  tss.ss0 = KDATA_SEL;
  cr3 = pa(cur_proc->page_dir)|0x07;
  
  create_proc("proc1", "TEST");
  create_proc("proc2", "TEST");

  asm volatile("movl %0, %%esp\n\t"
	       "movl %1, %%cr3\n"
	       "jmp start_proc\n\t"
	       ::"m"(cur_proc->hw_ctx.esp),"r"(cr3));
}
