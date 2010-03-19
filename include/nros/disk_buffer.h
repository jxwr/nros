#pragma once
#include <list.h>

#define DISK_BUFFER_BLOCK_SIZE 4096
#define DISK_BUFFER_DIRTY      1
#define DISK_BUFFER_BUSY       2

typedef struct db_node_s {
  link_t list;
  unsigned long flags;
  unsigned long block_num;
  void* buffer;
} db_node_t;

void disk_buffer_init();

db_node_t* disk_buffer_get(unsigned long block);

void disk_buffer_put(unsigned long block);

void diks_buffer_syc(unsigned long block);
