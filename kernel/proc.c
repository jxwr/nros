#include <nros/proc.h>
#include <nros/mm.h>
#include <stdio.h>
#include <string.h>

proc_t* proc_list;
proc_t* current_proc;

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
    if(page_dir[i] != 0)
      printf("%x ", page_dir[i]);
  }
}

void set_ptable_entry(unsigned long* ptable, int idx, unsigned long entry)
{
  ptable[idx] = entry;
}

void switch_proc();

pid_t create_proc()
{
  proc_t* proc = kmalloc(sizeof(proc_t));
  proc->name = "first proc";
  proc->page_dir = alloc_page_dir();
  proc->stack = proc->page_dir;

  page_t* page_table = alloc_pages(0);
  void* table_addr = page_to_vir(page_table);
  page_t* page = alloc_pages(0);
  void* page_addr = page_to_vir(page);
  set_ptable_entry(proc->page_dir, 0, pa(table_addr) | 0x03);
  set_ptable_entry(table_addr, 0, pa(page_addr) | 0x03);
  copy_kernel_vm(proc->page_dir);
  memcpy(page_addr, app, sizeof(app));

  link_init(&proc->list);
  proc_list = proc;

  switch_proc();

  return 0;
}


