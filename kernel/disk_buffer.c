/*
 * !!! need rewrite, didn't consider synchronization(no sleep yet)
 */

#include <nros/mm.h>
#include <nros/disk_buffer.h>
#include <nros/floppy.h>
#include <list.h>
#include <stdio.h>
#include <string.h>

typedef struct db_head_s {
  link_t db_node_list;
} db_head_t;

#define DISK_BUFFER_MOD 13

db_head_t db_head[DISK_BUFFER_MOD];


void disk_buffer_init()
{
  int i;
  for(i = 0; i < DISK_BUFFER_MOD; i++) {
    link_init(&db_head[i].db_node_list);
  }
}

static db_node_t* find_dnode(unsigned long block)
{
  int head_idx;
  db_node_t* dnode = NULL;
  db_node_t* dnode_found = NULL;
  
  head_idx = block % DISK_BUFFER_MOD;

  list_foreach_struct(dnode, &db_head[head_idx].db_node_list, list) {
    if(dnode && dnode->block_num == block) {
      dnode_found = dnode;
      break;
    }
  }

  return dnode_found;
}

/* synchronize specified block */
void disk_buffer_syc(unsigned long block)
{
  db_node_t* dnode_found = NULL;

  dnode_found = find_dnode(block);

  if(dnode_found) {
    int i, read_cnt;
    unsigned long sector_start;

    if(dnode_found->flags & DISK_BUFFER_BUSY) {
      /* need slepp here */
      printf("disk_buffer: block(%d) is busy.\n", block);
    }

    read_cnt = DISK_BUFFER_BLOCK_SIZE / FD_BLOCK_SIZE;
    sector_start = block * (DISK_BUFFER_BLOCK_SIZE / FD_SECTOR_SIZE);
    for(i = 0; i < read_cnt; i++) {
      memcpy(dma_buffer, (char*)dnode_found->buffer+i*FD_BLOCK_SIZE, FD_BLOCK_SIZE);
      fd_rw(DISK_WRITE, sector_start + i*(FD_BLOCK_SIZE/FD_SECTOR_SIZE));
    }
  }
}

/* !!! should consider synchronization, buffer lock or something */
db_node_t* disk_buffer_get(unsigned long block)
{
  int head_idx;
  db_node_t* dnode_found = NULL;
  
  head_idx = block % DISK_BUFFER_MOD;

  dnode_found = find_dnode(block);

  if(dnode_found) {
    if(dnode_found->flags & DISK_BUFFER_BUSY) {
      /* need slepp here */
      printf("disk_buffer: block(%d) is busy.\n", block);
    }
    dnode_found->flags |= DISK_BUFFER_BUSY;

    /* move the block to head */
    list_remove(&dnode_found->list);
    list_insert_after(&dnode_found->list, &db_head[head_idx].db_node_list);
    return dnode_found;
  }
  else {
    int i, read_cnt;
    unsigned long sector_start;
    db_node_t* dnode;
    
    dnode = kmalloc(sizeof(db_node_t));
    dnode->block_num = block;
    dnode->buffer = page_to_vir(alloc_pages(0));
    dnode->flags |= DISK_BUFFER_BUSY;
    list_insert_after(&dnode->list, &db_head[head_idx].db_node_list);

    read_cnt = DISK_BUFFER_BLOCK_SIZE / FD_BLOCK_SIZE;
    sector_start = block * (DISK_BUFFER_BLOCK_SIZE / FD_SECTOR_SIZE);
    for(i = 0; i < read_cnt; i++) {
      fd_rw(DISK_READ, sector_start + i*(FD_BLOCK_SIZE/FD_SECTOR_SIZE));
      memcpy((char*)dnode->buffer+i*FD_BLOCK_SIZE, dma_buffer, FD_BLOCK_SIZE);
    }

    return dnode;
  }
}

/*
 * clear busy flag of the block, 
 * should wake up tasks waiting on this block,
 * but task struct does not exist :(
 */
void disk_buffer_put(unsigned long block)
{
  db_node_t* dnode_found = NULL;
  
  dnode_found = find_dnode(block);
  if(dnode_found) {
    dnode_found->flags &= ~DISK_BUFFER_BUSY;
    disk_buffer_syc(block);
  }
}
