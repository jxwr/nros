#pragma once
#include <list.h>
#include <nros/mm.h>

/* hardware context */
typedef struct hw_ctx_s {
  unsigned long eax;
  unsigned long ebx;
  unsigned long ecx;
  unsigned long edx;
  unsigned long esi;
  unsigned long edi;
  unsigned long ebp;
  unsigned long esp;
  unsigned long eip;
  unsigned long eflags;
  unsigned long eip;
} hw_ctx_t;

/* process struct */
typedef struct proc_s {
  vm_info_t vm_info;
  void* stack;
  void* page_dir; /* x86, page directory virtual address */
  link_t list;
  char* name;
  hw_ctx_t hw_ctx;
} proc_t;

extern proc_t* current_proc;

typedef unsigned int pid_t;

void proc_init();

pid_t create_proc();
