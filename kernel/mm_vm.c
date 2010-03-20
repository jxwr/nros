#include <nros/mm.h>
#include <nros/proc.h>
#include <string.h>
#include <stdio.h>

#define VM_AREA_LIST(proc) \
  (proc->vm_info.vm_area_list)

/*
 * TODO: demand paging
 */
static void map_vm_area(proc_t* proc, vm_area_t* vma)
{
  unsigned long* pagedir_base = proc->page_dir;

  unsigned long start = vma->start & (~0xfff);
  unsigned long end = ((vma->end + 0xfff) & (~0xfff)) - 1;

  int pd_idx = start >> 22;
  int pd_idx_end = end >> 22;

  int pt_idx = (start >> 12) & 0x3ff;
  int pt_idx_end = ((end >> 12) & 0x3ff) 
    + (pd_idx_end - pd_idx) * PAGE_TABLE_ENTRY_CNT;
  
  while(pd_idx <= pd_idx_end) {
    unsigned long pagedir_entry;
    unsigned long* pagetable_base;

    if(!PAGE_PRESENT(pagedir_base[pd_idx])) {
      void* p = page_to_vir(alloc_page_zero());
      /* printf("ALLOC: pagetable:%x\n", p); */
      pagedir_base[pd_idx] = pa(p) | 0x03;
    }

    pagedir_entry = pagedir_base[pd_idx];
    pagetable_base = (unsigned long*)((pagedir_entry & (~0xfff)) + KVM_BASE);

    /* printf("page table base %x\n", pagetable_base); */
    do{
      if(!PAGE_PRESENT(pagetable_base[pt_idx])) {
	void* p = page_to_vir(alloc_page_zero());
	pagetable_base[pt_idx % PAGE_TABLE_ENTRY_CNT] = pa(p) | 0x03;
      }
      pt_idx++;
    }while(pt_idx % PAGE_TABLE_ENTRY_CNT != 0 && pt_idx <= pt_idx_end);
    pd_idx++;
  }
}

/*
 * find a free vm area with arbitrary start address,
 * and insert it into vm area list sorted by start address
 */
static vm_area_t* insert_free_vma(proc_t* proc, size_t size)
{
  unsigned long end = -1;
  vm_area_t* vma = NULL;
  vm_area_t* old_vma = NULL;

  for(vma = VM_AREA_LIST(proc); ;old_vma = vma, vma = vma->next) {
    if(vma == NULL || vma->start - end > size) {
      vm_area_t* vmb = kmalloc(sizeof(*vmb));
      vmb->next = vma;
      if(old_vma == NULL)
	VM_AREA_LIST(proc) = vmb;
      else
	old_vma->next = vmb;
      vmb->start = end + 1;
      vmb->end = end + size;
      return vmb;
    }
    end = vma->end;
  }
  return NULL;
}

void* do_vm_alloc(proc_t* proc, void* start, size_t size)
{
  vm_area_t* vma = NULL;
  vm_area_t* old_vma = NULL;
  unsigned long ustart = (unsigned long)start;
  unsigned long uend = ustart + size - 1;
  
  vma = VM_AREA_LIST(proc);

  /* empty list or first node */
  if(vma == NULL || uend < vma->start) {
    vm_area_t* vmb = kmalloc(sizeof(*vmb));
    vmb->next = vma;
    vmb->start = ustart;
    vmb->end = uend;
    VM_AREA_LIST(proc) = vmb;
    map_vm_area(proc, vmb);
    return start;
  }

  /* 
   * second node and on.
   * find a free area with start address 'start 
   */
  for(;; old_vma = vma, vma = vma->next) {
    if(vma != NULL && ustart > vma->start)
      continue;
    
    if(ustart > old_vma->end) {
      if((vma == NULL) || 
	 (vma != NULL && uend < vma->start)) {
	vm_area_t* vmb = kmalloc(sizeof(*vmb));
	vmb->next = vma;
	old_vma->next = vmb;
	vmb->start = ustart;
	vmb->end = uend;
	map_vm_area(proc, vmb);
	return start;
      }
    }
    break;
  }

  /* find a free area with arbitrary start address */
  vma = insert_free_vma(proc, size);
  if(vma == NULL)
    BUG_MSG("no more free vma.");
  map_vm_area(proc, vma);

  return (void*)vma->start;
}

void* alloc_page_dir()
{
  page_t* dir = alloc_page_zero();
  void* addr = page_to_vir(dir);

  return addr;
}


void unmap_vm_area(proc_t* proc, vm_area_t* vma)
{
  unsigned long* pagedir_base = proc->page_dir;

  unsigned long start = vma->start & (~0xfff);
  unsigned long end = ((vma->end + 0xfff) & (~0xfff)) - 1;

  int pd_idx = start >> 22;
  int pd_idx_end = end >> 22;

  int pt_idx = (start >> 12) & 0x3ff;
  int pt_idx_end = ((end >> 12) & 0x3ff) 
    + (pd_idx_end - pd_idx) * PAGE_TABLE_ENTRY_CNT;
  
  while(pd_idx <= pd_idx_end) {
    unsigned long pagedir_entry;
    unsigned long* pagetable_base;
    int entry_start = 0;
    int i, flag = 0;

    pagedir_entry = pagedir_base[pd_idx];
    pagetable_base = (unsigned long*)((pagedir_entry & (~0xfff)) + KVM_BASE);

    /* printf("free ptable base %x\n", pagetable_base); */

    entry_start = pt_idx % PAGE_TABLE_ENTRY_CNT;
    do{
      unsigned long pagetable_entry;
      unsigned long* page_base;
      
      pagetable_entry = pagetable_base[pt_idx % PAGE_TABLE_ENTRY_CNT];
      pagetable_base[pt_idx % PAGE_TABLE_ENTRY_CNT] = 0;
      page_base = (unsigned long*)((pagetable_entry & (~0xfff)) + KVM_BASE);
      free_pages(vir_to_page(page_base), 0);

      pt_idx++;
    }while((pt_idx % PAGE_TABLE_ENTRY_CNT != 0) && pt_idx <= pt_idx_end);

    /* free pagetable page if none page in it is mapped  */    
    for(i = 0; i < PAGE_TABLE_ENTRY_CNT; i++) {
      if(pagetable_base[i] != 0) {
	flag++;
      }
    }
    if(flag == 0) {
      free_pages(vir_to_page(pagetable_base), 0);
    }

    pd_idx++;
  }
}

/*
 * free the vm_area of 'proc with start address == p,
 * if not find, do nothing
 */
void vm_free_area(proc_t* proc, void* p)
{
  vm_area_t* vma = NULL;
  vm_area_t* old_vma = NULL;
  unsigned long start = (unsigned long)p;

  for(vma = VM_AREA_LIST(proc); vma != NULL; old_vma = vma, vma = vma->next) {
    if(start == vma->start) {
      unmap_vm_area(proc, vma);
      if(old_vma == NULL)
	VM_AREA_LIST(proc) = vma->next;
      else
	old_vma->next = vma->next;

      kfree(vma);
    }
  }
}

/*
 * free all vm_areas within the 'proc
 */
void do_vm_free(proc_t* proc)
{
  vm_area_t* vma;

  vma = VM_AREA_LIST(proc);
  VM_AREA_LIST(proc) = NULL;
  
  for(; vma != NULL; vma = vma->next) {
    vm_free_area(proc, vma);
  }
}

static void print_vma_list(proc_t* proc)
{
  vm_area_t* vma = proc->vm_info.vm_area_list;
  
  if(vma != NULL)
  for(; vma != NULL; vma = vma->next) {
    printf("start:%x,end:%x\n", vma->start, vma->end);
  }
}

void test_vm()
{
  proc_t proc;
  void* addr;

  proc.vm_info.vm_area_list = NULL;
  proc.page_dir = alloc_page_dir();
  //  addr = do_vm_alloc(&proc, (void*)0x1000, 4<<20);
  //addr = do_vm_alloc(&proc, (void*)0x3000, 0x1050);
  //  addr = do_vm_alloc(&proc, (void*)0x5000, 0x1000);
  //  addr = do_vm_alloc(&proc, (void*)0x2500, 0x100);
  //  addr = do_vm_alloc(&proc, (void*)0x4000, 0x500);
  
  addr = do_vm_alloc(&proc, (void*)((4 << 20)-0x2000), 5 << 20);
  vm_free_area(&proc, (void*)((4 << 20)-0x2000));
  vm_free_area(&proc, (void*)0x1000);
  //vm_free_area(&proc, addr);
  do_vm_free(&proc);
  print_vma_list(&proc);
}
