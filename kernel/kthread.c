#include <list.h>
#include <mm.h>

typedef struct kthread_s {
  char* name;
  char* stack;

  link_t ktd_list;
  
  /* regs */
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
  unsigned int esi;
  unsigned int edi;
  unsigned int ebp;
  unsigned int esp;
  unsigned int eip;

  unsigned short ss;
  unsigned short cs;
  unsigned short ds;
  unsigned short es;
  unsigned short fs;
} kthread_t;

int kthead_create(void (*func)(), char* name)
{
  kthead_t* ktd;

  ktd = kmalloc(sizeof(kthread_t));
  ktd->name = name;
  ktd->stack = alloc_pages(0);
}
