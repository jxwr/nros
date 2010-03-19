#pragma once
#include <list.h>
#include <nros/mm.h>

typedef struct proc_s {
  vm_info_t vm_info;

  void* stack;
  void* page_dir; /* x86, page directory virtual address */
  link_t list;
  char* name;
} proc_t;

extern proc_t* current_proc;

typedef unsigned int pid_t;

pid_t create_proc();
