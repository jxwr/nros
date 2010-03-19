#include <stdio.h>
#include <string.h>
#include <nros/macro.h>
#include <nros/mm.h>

#define MAX_ORDER 11

extern int kpagedir;
extern int _img_end;
unsigned long img_end;

page_t* mem_map;
page_t* mem_map_end;

/* number of all pages */
unsigned long page_count;


typedef struct free_area_s {
  link_t free_list;
  unsigned long* map;
} free_area_t;

/* free pages */
unsigned long free_page_count;

free_area_t free_area[MAX_ORDER];

static void set_page_table(unsigned long page_table_addr,
		    unsigned long phy_addr,
		    unsigned long flags)
{
  int i;
  unsigned long* page_table_entry;
  
  page_table_entry = (unsigned long*)page_table_addr;
  
  for(i = 0; i < PAGE_TABLE_ENTRY_CNT; i++) {
    *page_table_entry = phy_addr | flags;
    page_table_entry++;
    phy_addr += PAGE_SIZE;
  }
}

/*
 * map all physical memory to kernel space (0xc000,0000)
 * we have map the first 4M to kernel space already
 */
static void map_all_memory(unsigned long page_cnt)
{
  unsigned long phy_addr;
  unsigned long max_phy_addr;
  unsigned long* pagedir_entry;

  phy_addr = 4 << 20;   /* from 4M */
  max_phy_addr = page_cnt << PAGE_SHIFT;
  pagedir_entry = (unsigned long*)((unsigned long)&kpagedir + 0x0c04); /* from second entry */
  img_end = (unsigned long)&_img_end;

  img_end = PAGE_ALIGN(img_end);
  
  while(phy_addr < max_phy_addr) {
    *pagedir_entry = pa(img_end) | 0x07;
    set_page_table(img_end, phy_addr, 0x07);
    phy_addr += PAGE_SIZE * PAGE_TABLE_ENTRY_CNT;
    pagedir_entry++;
    img_end += PAGE_SIZE;
  }
}

static int change_and_test_bit(unsigned long idx, void* map)
{
  char* m = (char*)map;
  int the_byte = idx >> 3;
  int the_bit = idx % 8;
  int ret = (*(m + the_byte) >> the_bit) & 1;

  *(m + the_byte) ^= 1 << the_bit;

  return ret == 0;
}


void free_pages(page_t* page, unsigned long order)
{
  unsigned long page_idx, bitmap_idx, mask;
  page_t* buddy;
  free_area_t* area;
  
  page_idx = page - mem_map;
  bitmap_idx = page_idx >> (1 + order);
  area = free_area + order;
  
  mask = (~0UL) << order;
  
  /* free_pages = free_pages + (-mask)*/
  free_page_count -= mask;
  
  while(order < MAX_ORDER -1) {
    /* 
     * if true, means its buddy is in use, 
     * just insert it at current order free_list
     */
    if(change_and_test_bit(bitmap_idx, area->map)) {
      break;
    }

    /* 
     * otherwise, we shoule merge it with its buddy,
     * then check the up one level
     */

    /* remove the buddy in this level */
    buddy = mem_map + (page_idx ^ -mask);
    list_remove(&buddy->list);

    area++;
    order++;
    bitmap_idx >>= 1;
    mask <<= 1;
    page_idx &= mask;
  }
  
  list_insert_after(&(mem_map + page_idx)->list, &area->free_list);
}

#define mark_used(idx, order, area) \
  change_bit((idx)>>(1+(order)),(area)->map)

static void change_bit(unsigned long idx, void* map)
{
  char* m = (char*)map;
  int the_byte = idx >> 3;
  int the_bit = idx % 8;
  *(m + the_byte) ^= 1 << the_bit;
}

static inline page_t* expand(page_t* page, unsigned long idx, 
			     int low, int high, free_area_t* area)
{
  unsigned long size = 1 << high;

  while(high > low) {
    area--;
    high--;
    size >>= 1;
    list_insert_after(&page->list, &area->free_list);
    mark_used(idx, high, area);
    idx += size;
    page += size;
  }
  
  return page;
}
page_t* _alloc_pages(unsigned long order)
{
  free_area_t* area = free_area + order;
  link_t* head;
  link_t* curr;
  unsigned long curr_order = order;

  do {
    head = &area->free_list;
    curr = head->next;
    if(curr != head) {
      unsigned long idx;
      page_t* page;
      
      page = link_to_struct(curr, page_t, list);
      list_remove(curr);

      idx = page - mem_map;
      if(curr_order != MAX_ORDER - 1)
	mark_used(idx, curr_order, area);
      free_page_count -= 1UL << order;
      page = expand(page, idx, order, curr_order, area);
      return page;
    }
    curr_order++;
    area++;
  } while(curr_order <= MAX_ORDER - 1);
   
  return NULL;
}

page_t* alloc_pages(unsigned long order)
{
  page_t* page = NULL;

  if(order > MAX_ORDER)
    BUG_MSG("alloc_pages, order > MAX_ORDER");

  page = _alloc_pages(order);
  if(page == NULL)
    BUG_MSG("memory alloc failed");
  return page;
}

static void buddy_system_init(unsigned long page_cnt)
{
  int i,j,idx;
  page_t* page;
  
  img_end = LONG_ALIGN(img_end);
  
  /* init bitmap of each free_area */
  for(i = 0; i < MAX_ORDER; i++) {
    unsigned long bitmap_size;
    link_init(&free_area[i].free_list);
    free_area[i].map = (unsigned long*)img_end;
    bitmap_size = (page_cnt-1) >> (i+4);
    bitmap_size = LONG_ALIGN(bitmap_size+1);
    memset(free_area[i].map, 0, bitmap_size);
    img_end += bitmap_size;
  }

  /* init some global variables */
  mem_map = (page_t*)img_end;
  mem_map_end = mem_map + page_cnt;
  img_end += page_cnt * sizeof(page_t);
  page_count = page_cnt;
  free_page_count = 0;

  /* set page flags */
  page = mem_map;
  for(j = 0; j < page_cnt; j++) {
    page->flags = j;
    link_init(&page->list);
    page++;
  }

  /*
   * free all page right after 'end' to the freelist
   * we can regard this as the initialization of freelist
   */
  img_end = PAGE_ALIGN(img_end + 0xfff);
  page = vir_to_page(img_end);
  idx = page - mem_map;

  /* printf("end_idx:%d\n",idx); */
  for(; idx < page_cnt; idx++) {
    free_pages(page++, 0);
  }
}

void mm_init(unsigned long mem_size)
{
  unsigned long page_cnt;

  page_cnt = mem_size >> 2;
  map_all_memory(page_cnt);

  buddy_system_init(page_cnt);
  kmalloc_init();
}

/* test */
void print_free_area()
{
  int i;

  for(i = 0; i < MAX_ORDER; i++) {
    page_t* page;
    printf("order:%d\n",i);
    list_foreach_struct(page, &free_area[i].free_list, list) {
      unsigned long idx;
      idx = (page - mem_map);
      printf("->(idx:%d)", idx);
    }
    printf("\n");
  }
}

