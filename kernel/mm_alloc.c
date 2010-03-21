#include <nros/mm.h>
#include <stdio.h>


/* memory area in node */
typedef struct marea_s {
  unsigned long size;  /* alloc size + struct size */
  struct marea_s* next;
} marea_t;

typedef struct malloc_node_s {
  unsigned long order;
  marea_t* free_area_list;
  marea_t* used_area_list;
  struct malloc_node_s* next;
} malloc_node_t;

/* 
 * global variables used for kmalloc
 */
malloc_node_t* malloc_list;

/*
 * alloc pages for this node and initialize structs within this area
 */
static malloc_node_t* new_malloc_node(unsigned long order)
{
  page_t* page = alloc_pages(order);
  malloc_node_t* mnp;
  marea_t* ma;

  mnp = page_to_vir(page);
  mnp->order = order;
  mnp->free_area_list = (void*)((unsigned long)mnp + sizeof(malloc_node_t));
  mnp->used_area_list = 0;
  mnp->next = NULL;

  ma = mnp->free_area_list;
  ma->size = (PAGE_SIZE << order) - sizeof(malloc_node_t);
  ma->next = NULL;

  return mnp;
}

void kmalloc_init()
{
  /* alloc a page */
  malloc_list = new_malloc_node(0);
}

/*
 * layout:
 *
 *  malloc_list----
 *                |
 *  ---------------
 *  |
 *  |page(order)
 *  ---------------------------------------------------------
 *  |malloc_node_t|marea_t,space|marea_t,space| ...         |
 *  ---------------------------------------------------------
 *          |
 *  ---------
 *  |
 *  |page(order)
 *  ---------------------------------------------------------
 *  |malloc_node_t|marea_t,space|marea_t,space| ...         |
 *  ---------------------------------------------------------
 *           |
 *  ----------
 *  |
 * page(order)
 * ...
 *
 */
void* kmalloc(size_t size)
{
  unsigned long order;
  malloc_node_t* mnp;
  size_t real_size;

 retry:
  order = size >> PAGE_SHIFT;
  real_size = LONG_ALIGN(size + sizeof(marea_t));

  BUG_ON(malloc_list == NULL);

  for(mnp = malloc_list; mnp != NULL; mnp = mnp->next) {
    if(mnp->order >= order) {
      marea_t* ma = NULL;
      marea_t* ma_last = NULL;

      for(ma = mnp->free_area_list; ma != NULL; ma_last = ma, ma = ma->next) {
	if(ma->size >= real_size) {
	  size_t old_size = ma->size;
	  void* old_next = ma->next;
	  marea_t* nma;
	  size_t rest;

	  /* resize this marea */
	  ma->size = real_size;

	  /* move it to used_area_list */
	  ma->next = mnp->used_area_list;
	  mnp->used_area_list = ma;

	  rest = old_size - real_size;

	  if(rest > sizeof(marea_t)) {
	    nma = (marea_t*)((unsigned long)ma + ma->size);

	    /* printf("ma:%x,nma:%x\n",ma,nma); */

	    nma->size = rest;
	    nma->next = old_next;

	    if(ma_last == NULL) {
	      mnp->free_area_list = nma;
	    }
	    else
	      ma_last->next = nma;
	  }
	  else {
	    if(ma_last == NULL)
	      mnp->free_area_list = NULL;
	    else
	      ma_last->next = NULL;
	  }

	  return (void*)((unsigned long)ma + sizeof(marea_t));
	}
      }
    }
  }
  
  /* can not find proper space, add a new malloc_node */
  if((mnp = new_malloc_node(order))) {
    mnp->next = malloc_list;
    malloc_list = mnp;
    goto retry;
  }
  
  return NULL;
}

void kfree(void* p)
{
  malloc_node_t* mnp = NULL;
  malloc_node_t* mnp_last = NULL;
  unsigned long up = (unsigned long)p;

  /*
   * find marea_t which hold 'p first, move it to the free_area_list, 
   * if it is adjacent to the previous node or the next, merge them.
   */
  for(mnp = malloc_list; mnp != NULL; mnp_last = mnp, mnp = mnp->next) {
    marea_t* ma = NULL;
    marea_t* ma_last = NULL;

    for(ma = mnp->used_area_list; ma != NULL; ma_last = ma, ma = ma->next) {
      if(up == (unsigned long)ma + sizeof(marea_t)) {
	marea_t* mb = NULL;
	marea_t* mb_last = NULL;

	/* remove this marea from used_area_list */
	if(ma_last == NULL) {
	  mnp->used_area_list = mnp->used_area_list->next;
	}
	else {
	  ma_last->next = ma->next;
	}

	/* add 'ma to free_area_list */
	for(mb = mnp->free_area_list; ; mb_last = mb, mb = mb->next) {
	  if(mb != NULL && ma >= mb)
	    continue;

	  /* be adjacent to the previous? */
	  if(mb_last == NULL) {
	    ma->next = mnp->free_area_list;
	    mnp->free_area_list = ma;
	  }
	  else if(((unsigned long)mb_last + mb_last->size) == (unsigned long)ma) {
	    mb_last->size += ma->size;
	    ma = mb_last;
	  }

	  /* be adjacent to the next? */
	  if((unsigned long)ma + ma->size == (unsigned long)mb) {
	    ma->size += mb->size;
	    ma->next = mb->next;

	    if(ma != mb_last) {
	      mb_last->next = ma;
	    }
	  }

	  /*
	   * page recycle 
	   * remain at least one malloc_node 
	   */
	  if(mnp_last != NULL) { 
	    ma = mnp->free_area_list;
	    if((ma->size + sizeof(malloc_node_t)) == PAGE_SIZE << mnp->order) {
	      mnp_last->next = mnp->next;
	      free_pages(vir_to_page(mnp), mnp->order);
	    }
	  }

	  return;
	}
	/* printf("free %x\n", p); */
      }
    }
  }

  BUG_MSG("kfree: illegal address(%x)\n", p);
}

void print_malloc_list()
{
  marea_t* ma;
  malloc_node_t* mnp;

  for(mnp = malloc_list; mnp != NULL; mnp = mnp->next) {
    printf("malloc_list: ----------------\n");
    printf(" free_area_list:\n");
    for(ma = mnp->free_area_list;
	ma != NULL;
	ma = ma->next) {
      printf("  ->(p:%x, size:%d)\n", ma, ma->size);
    }
    
    printf(" used_area_list:\n");
    for(ma = mnp->used_area_list;
	ma != NULL;
	ma = ma->next) {
      printf("  ->(p:%x, size:%d)\n", ma, ma->size);
    }
  }
}


void kmalloc_test()
{
  void* r = kmalloc(0x300);
  void* p = kmalloc(0x100);
  //  alloc_pages(0);
  void* t = kmalloc(0x1000);
  void* s = kmalloc(0x1234);
  unsigned long* q = kmalloc(0x200);

  *q = 0x12345678;
  print_malloc_list();
  for(;;);
  printf("p:%x  ", p);
  printf("q:%x  ", q);
  printf("r:%x  ", r);
  printf("s:%x  ", s);
  printf("t:%x\n", t);

  kfree(s);
  kfree(r);
  kfree(p);
  kfree(q);
  kfree(t);
  
  print_malloc_list();
}


