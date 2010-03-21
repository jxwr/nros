#pragma once
#include <list.h>
#include <nros/mm.h>

/* hardware context */
typedef struct hw_ctx_s {
  unsigned long esp;
  unsigned long eip;
} hw_ctx_t;

typedef unsigned int pid_t;

/* process struct */
typedef struct proc_s {
  pid_t pid;
  vm_info_t vm_info;
  void* user_stack;
  void* kernel_stack;
  void* page_dir; /* x86, page directory virtual address */
  link_t list;
  char* name;
  hw_ctx_t hw_ctx;
} proc_t;

extern proc_t* cur_proc;
extern link_t proc_list;


void proc_init();

pid_t create_proc();

void schedule();
