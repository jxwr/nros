#include <nros/proc.h>
#include <nros/mm.h>
#include <stdio.h>
#include <list.h>
#include <string.h>

link_t proc_list;
proc_t* cur_proc;
pid_t pid_idx;

unsigned long kpagedir;

unsigned int app[] = {
 0x4bbe,0x0000,0xb800,0x0001,0x0000,0x40cd,0x03b8,0x0000
,0xcd00,0xbe40,0x0070,0x0000,0x01b8,0x0000,0xcd00,0xb840
,0x0003,0x0000,0x40cd,0x96be,0x0000,0xb800,0x0001,0x0000
,0x40cd,0x0cb8,0x0000,0xcd00,0xeb40,0x54fe,0x6568,0x6572
,0x6120,0x6572,0x6f20,0x6c6e,0x2079,0x7774,0x206f,0x6f63
,0x6d6d,0x6e61,0x7364,0x6620,0x726f,0x6e20,0x776f,0x003a
,0x2020,0x2820,0x2931,0x7370,0x2020,0x2020,0x6420,0x7369
,0x6c70,0x7961,0x6120,0x6c6c,0x7220,0x6e75,0x696e,0x676e
,0x7420,0x7361,0x006b,0x2020,0x2820,0x2932,0x6568,0x706c
,0x2020,0x6920,0x2774,0x2073,0x656d,0x0000
};

static void copy_kernel_vm(unsigned long* page_dir)
{ 
  /* 0x0c00 */
  int i;

  for(i = 0x300; i < 1024; i++) {
    page_dir[i] = (&kpagedir)[i];
  }
}

/* used for test */
void __attribute__((regparm(3))) proc_app()
{
  asm volatile("movl $0x12345678, %eax\n");
  asm volatile("movl $0x12345678, %ebx\n");
  asm volatile("1: jmp 1b\n");
}

/* copy app to proc space */
void copy_app(proc_t* proc)
{
  unsigned long cr3 = pa(proc->page_dir)|0x03;
  asm volatile("movl %0, %%ebx\n" 
	       "movl %%ebx, %%cr3":: "m"(cr3));
  memcpy((void*)0x110000, &proc_app, 0x200);
}

extern void start_proc();

pid_t create_proc(char* name)
{
  proc_t* proc = kmalloc(sizeof(proc_t));
  proc->name = name;
  proc->pid = pid_idx++;
  proc->page_dir = alloc_page_dir();
  printf("dir:%x\n",proc->page_dir);
  copy_kernel_vm(proc->page_dir);
  do_vm_alloc(proc, (void*)0x110000, 0x2000);
  proc->user_stack = (void*)(0x110000-1);
  proc->hw_ctx.esp = (unsigned long)page_to_vir(alloc_pages(1)) + 0x2000-4;
  proc->hw_ctx.esp0 = proc->hw_ctx.esp;
  proc->hw_ctx.eip = (unsigned long)start_proc;
  printf("esp0:%x\n", proc->hw_ctx.esp);
  copy_app(proc);
  link_init(&proc->list);
  list_insert_after(&proc->list, &proc_list);
  if(cur_proc == NULL)
    cur_proc = proc;

  return 0;
}

void proc_init()
{
  pid_idx = 0;
  cur_proc = NULL;
  link_init(&proc_list);
  create_proc("first proc");
  tss.esp0 = cur_proc->hw_ctx.esp;
  tss.ss0 = KDATA_SEL;
  create_proc("second proc");
  asm volatile("movl %0, %%esp\n\t"
	       "jmp start_proc\n\t"
	       ::"m"(cur_proc->hw_ctx.esp));
}
