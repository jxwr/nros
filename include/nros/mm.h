#pragma once
#include <nros/common.h>
#include <ctype.h>
#include <string.h>
#include <list.h>


typedef struct page_s {
  link_t list;
  unsigned long flags;
} page_t;

struct vm_info_s;

typedef struct vm_area_s {
  struct vm_info_s* vm_info;
  struct vm_area_s* next;
  unsigned long start;
  unsigned long end;
} vm_area_t;

typedef struct vm_info_s {
  vm_area_t* vm_area_list;
} vm_info_t;

extern page_t* mem_map;
extern page_t* mem_map_end;
extern unsigned long page_count;
extern unsigned long free_page_count;

#define PAGE_SHIFT 12
#define PAGE_SIZE  4096

#define PAGE_TABLE_ENTRY_CNT 1024
#define PAGE_DIR_ENTRY_CNT   1024

#define PAGE_PRESENT(page) (page & 1)

#define PAGE_ALIGN(x) ALIGN(x,0x1000)

#define page_to_vir(page) (void*)(((page-mem_map) << PAGE_SHIFT) + KVM_BASE)
#define vir_to_page(addr) (mem_map + ((unsigned long)pa(addr) >> PAGE_SHIFT))

/* mm.c */
void mm_init(unsigned long mem_size);

page_t* alloc_pages(unsigned long order);

static inline page_t* alloc_page_zero() {
  page_t* page = alloc_pages(0);
  void* addr = page_to_vir(page);

  memset(addr, 0, PAGE_SIZE);
  return page;
}

void free_pages(page_t* page, unsigned long order);

void print_free_area();

/* mm_alloc.c */
void kmalloc_init();

void* kmalloc(size_t size);

void kfree(void* p);

void kmalloc_test();

void print_malloc_list();

/* mm_vm.c */
struct proc_s;

void* alloc_page_dir();

void* do_vm_alloc(struct proc_s* proc, void* start, size_t size);

void do_vm_free(struct proc_s* proc);

void vm_free_area(struct proc_s* proc, void* p);
