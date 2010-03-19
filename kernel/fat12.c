#include <nros/disk_buffer.h>
#include <nros/common.h>
#include <stdio.h>
#include <string.h>

#define SECTOR_SIZE       512
#define FAT_START_SECT    1
#define ROOT_START_SECT   19
#define DATA_START_SECT   33

#define FAT12_READ        0
#define FAT12_WRITE       1

#define FAT12_INFO_READ   0
#define FAT12_INFO_SYC    1

static char fat[8*512];
static char root[14*512];

/* return sector from disk buffer cache */
static void* read_sector(unsigned short sect_num,unsigned long* block_nump)
{
  int block_num;
  db_node_t* dnode;

  /* which block to get */
  block_num = sect_num / (DISK_BUFFER_BLOCK_SIZE / SECTOR_SIZE);

  /* which sector do we want */
  sect_num %= (DISK_BUFFER_BLOCK_SIZE / SECTOR_SIZE);

  dnode = disk_buffer_get(block_num);
  *block_nump = block_num;

  return ((char*)dnode->buffer + SECTOR_SIZE*sect_num);
}

static void fat12_set_info(int cmd)
{
  db_node_t* dnode[5];

  dnode[0] = disk_buffer_get(0); /* 0 -7  */
  dnode[1] = disk_buffer_get(1); /* 8 -15 */
  dnode[2] = disk_buffer_get(2); /* 16-23 */
  dnode[3] = disk_buffer_get(3); /* 24-31 */
  dnode[4] = disk_buffer_get(4); /* 32-39 */

  if(cmd == FAT12_INFO_READ) {
    /* 
     * fill FAT, 8 sectors 
     * dnode[0] - (1-7)
     * dnode[1] - (8)
     */
    memcpy(fat, 
	   (char*)dnode[0]->buffer + SECTOR_SIZE, 
	   DISK_BUFFER_BLOCK_SIZE - SECTOR_SIZE);
    memcpy(fat + (DISK_BUFFER_BLOCK_SIZE - SECTOR_SIZE), 
	   (char*)dnode[1]->buffer, 
	   SECTOR_SIZE);

    /*
     * fill ROOT, 14 sectors
     * dnode[2] - (19-23)
     * dnode[3] - (24-31)
     * dnode[4] - (32)
     */
    memcpy(root, 
	   (char*)dnode[2]->buffer + 3*SECTOR_SIZE, 
	   DISK_BUFFER_BLOCK_SIZE - 3*SECTOR_SIZE);
    memcpy(root + (DISK_BUFFER_BLOCK_SIZE - 3*SECTOR_SIZE), 
	   (char*)dnode[3]->buffer,
	   DISK_BUFFER_BLOCK_SIZE);
    memcpy(root + DISK_BUFFER_BLOCK_SIZE * 2 - 3*SECTOR_SIZE,
	   (char*)dnode[4]->buffer,
	   SECTOR_SIZE);
  }
  else if(cmd == FAT12_INFO_SYC) {
    memcpy((char*)dnode[0]->buffer + SECTOR_SIZE, 
	   fat, 
	   DISK_BUFFER_BLOCK_SIZE - SECTOR_SIZE);
    memcpy((char*)dnode[1]->buffer, 
	   fat + (DISK_BUFFER_BLOCK_SIZE - SECTOR_SIZE), 
	   SECTOR_SIZE);

    memcpy((char*)dnode[2]->buffer + 3*SECTOR_SIZE, 
	   root, 
	   DISK_BUFFER_BLOCK_SIZE - 3*SECTOR_SIZE);
    memcpy((char*)dnode[3]->buffer,
	   root + (DISK_BUFFER_BLOCK_SIZE - 3*SECTOR_SIZE), 
	   DISK_BUFFER_BLOCK_SIZE);
    memcpy((char*)dnode[4]->buffer,
	   root + DISK_BUFFER_BLOCK_SIZE * 2 - 3*SECTOR_SIZE,
	   SECTOR_SIZE);
  }

  disk_buffer_put(0);
  disk_buffer_put(1);
  disk_buffer_put(2);
  disk_buffer_put(3);
  disk_buffer_put(4);
}

static int get_free_fat_entry()
{
  int i;

  for(i = 0; i < 2730; i++) {
    int c = i * 3 / 2;
    unsigned short cluster = *(unsigned short*)(fat + c);

    if(i & 1) {
      cluster >>= 4;
    }
    cluster &= 0x0fff;

    /* printf("%x\t  ", cluster); */
    if(cluster == 0) {
      /* printf("\ni:%x", i); */
      if(i & 1) {
	*(unsigned short*)(fat + c) &= 0x000f;
	*(unsigned short*)(fat + c) |= 0xff80;	
      }
      else {
	*(unsigned short*)(fat + c) &= 0xf000;
	*(unsigned short*)(fat + c) |= 0x0ff8;	
      }
      return i;
    }
  }
  return -1;
}

static void fat12_create_aux(char* dir_buf, char* filename)
{
  int i, j;
  char* dp1 = dir_buf;
  char* dp2 = dp1;
  char fname[9];
  
  memset(fname, ' ', 8);
  strncpy(fname, filename, 8);
  fname[8] = 0;

  for(i = 0; i < 224; i++) {
    if(strncmp(dp1, fname, 8) == 0) {
      return;
    }
    dp1 += 32;
  }

  for(j = 0; j < 224; j++) {
    if(dp2[0] == 0) {
      memset(dp2, ' ', 11);
      memcpy(dp2, filename, strlen(filename));
      *(unsigned short*)(dp2+26) = get_free_fat_entry();
      *(unsigned long*)(dp2+28) = 0;
      break;
    }
    dp2 += 32;
  }
}

static void resize(char* dp, unsigned long newsize)
{
  unsigned short cluster = *(unsigned short*)(dp+26);

  unsigned long filesize = *(unsigned long*)(dp+28);
  int sect_needed = (newsize + SECTOR_SIZE) / SECTOR_SIZE - (filesize + SECTOR_SIZE) / SECTOR_SIZE;

  *(unsigned long*)(dp+28) = newsize;

  if(sect_needed == 0)
    return;
    
  while(1) {
    unsigned short fat_entry_idx = cluster * 3 / 2;
    unsigned short* next_clusterp = (unsigned short*)(fat + fat_entry_idx);
    unsigned short next_cluster = *next_clusterp;

    if(cluster & 1) {
      next_cluster >>= 4;
    }
    next_cluster &= 0xfff;
      
    if(next_cluster >= 0xff8) {
      int i;

      for(i = 0; i < sect_needed; i++) {
	next_cluster = get_free_fat_entry();
	if(cluster & 1) {
	  *next_clusterp &= 0x000f;
	  *next_clusterp |= next_cluster << 4;
	}
	else {
	  *next_clusterp &= 0xf000;
	  *next_clusterp |= next_cluster & 0x0fff;
	}
	cluster = next_cluster;
	fat_entry_idx = cluster * 3 / 2;
	next_clusterp = (unsigned short*)(fat + fat_entry_idx);
	next_cluster = *next_clusterp;
      }
      fat12_set_info(FAT12_INFO_SYC);
      return;
    }
    cluster = next_cluster;
  }
}


void fat12_remove_aux(char* dir_buf, char* filename)
{
  char* dp = dir_buf;
  int i;
  char fname[9];
  
  memset(fname, ' ', 8);
  strncpy(fname, filename, 8);
  fname[8] = 0;

  for(i = 0; i < 224; i++) {
    if(strncmp(dp, fname, 8) == 0) {
      unsigned short cluster = *(unsigned short*)(dp+26);
      int j;

      while(1) {
	unsigned short fat_entry_idx = cluster * 3 / 2;
	unsigned short* next_clusterp = (unsigned short*)(fat + fat_entry_idx);
	unsigned short next_cluster = *next_clusterp;
	
	if(cluster & 1) {
	  next_cluster >>= 4;
	  *next_clusterp &= 0x000f;
	}
	else {
	  next_cluster &= 0xfff;
	  *next_clusterp &= 0xf000;
	}

	if(next_cluster >= 0xff8)
	  break;
	cluster = next_cluster;
      }
      for(j = 0; j < 32; j++)
	dp[j] = 0;
    }

    dp += 32;
  }
}


static void fat12_rw_aux(int cmd, char* dir_buf, char* filename, int offset, void* buf, size_t len)
{
  char* rp = dir_buf;
  int i;
  char fname[9];
  
  memset(fname, ' ', 8);
  strncpy(fname, filename, 8);
  fname[8] = 0;

  for(i = 0; i < 224; i++) {
    if(strncmp(rp, fname, 8) == 0) {
      int start_sect = offset / SECTOR_SIZE;
      int end_sect = (offset + len) / SECTOR_SIZE;
      /* data head */
      int start_byte = offset % SECTOR_SIZE;
      /* data tail */
      int end_byte = (offset + len) % SECTOR_SIZE;
      /* counter */
      int curr_sect = 0;

      unsigned long filesize = *(unsigned long*)(rp+28);
      
      unsigned short first_cluster = *(unsigned short*)(rp+26);
      /* printf("first cluster: %d\n", first_cluster); */
      unsigned short cluster = first_cluster;
      char* bufp = buf;

      if(offset + len > filesize)
	resize(rp, offset + len);

      do{
	unsigned short sect_num = cluster + 31;
	unsigned short fat_idx = cluster * 3 / 2;
	unsigned short next_cluster = *(unsigned short*)(fat + fat_idx);
	char* sect_buf;
	
	if(cluster & 1) {
	  next_cluster >>= 4;
	}
	next_cluster &= 0x0fff;

	if(curr_sect >= start_sect && curr_sect <= end_sect) {
	  unsigned long* block_num = NULL;
	  sect_buf = read_sector(sect_num, block_num);

	  if(curr_sect == start_sect) {
	    int cpysize =  (len + start_byte) > SECTOR_SIZE ? (SECTOR_SIZE - start_byte) : len;
	    if(cmd == FAT12_READ)
	      memcpy(bufp, sect_buf + start_byte, cpysize);
	    else if(cmd == FAT12_WRITE)
	      memcpy(sect_buf + start_byte, bufp, cpysize);
	    else
	      BUG_MSG("FAT12 cmd error.\n");
	    bufp += cpysize;
	  }
	  else if(curr_sect < end_sect) {
	    if(cmd == FAT12_READ)
	      memcpy(bufp, sect_buf, SECTOR_SIZE);
	    else if(cmd == FAT12_WRITE)
	      memcpy(sect_buf, bufp, SECTOR_SIZE);
	    else
	      BUG_MSG("FAT12 cmd error.\n");
	    bufp += SECTOR_SIZE;
	  }
	  else if(curr_sect == end_sect) {
	    if(cmd == FAT12_READ)
	      memcpy(bufp, sect_buf, end_byte);
	    else if(cmd == FAT12_WRITE)
	      memcpy(sect_buf, bufp, end_byte);
	    else
	      BUG_MSG("FAT12 cmd error.\n");
	  }
	  disk_buffer_put(*block_num);
	}

	if(next_cluster >= 0xff8)
	  break;

	curr_sect++;
	cluster = next_cluster;
      } while(1);
    }
    rp += 32;
  }
  /*  
      if(i == 224)
      panic("file not found.\n");
  */
}

static void print_dir()
{
  int i;
  char* rp = root;

  for(i = 0; i < 224; i++) {
    char filename[12];

    memset(filename, 0, 12);
    memcpy(filename, rp, 11);

    if(filename[0] == 0) {
      rp += 32;
      continue;
    }

    printf("filename:%s", filename);
    printf(" size:%d,", *(int*)(rp+28));
    printf(" first cluster:%d\n", *(short*)(rp+26));
    rp += 32;
  }
}


/* no implementation */
void fat12_open(char* path)
{

}

/* 
 * read 'size bytes of the file in the root directory from 'offset into 'buffer,
 * if filesize is less than 'offset + 'size, filesize will change to that large
 */
void fat12_read(char* filename, int offset, void* buffer, size_t size)
{
  fat12_rw_aux(FAT12_READ, root, filename, offset, buffer, size);
}

/* 
 * write 'size bytes of the 'buffer in the file
 */
void fat12_write(char* filename, int offset, void* buffer, size_t size)
{
  fat12_rw_aux(FAT12_WRITE, root, filename, offset, buffer, size);
}

/*
 * create a new file in the root directory with size 0
 */
void fat12_create(char* filename)
{
  fat12_create_aux(root, filename);
}

/*
 * remove file
 */
void fat12_remove(char* filename)
{
  fat12_remove_aux(root, filename);
}

/*
 * init FAT12 filesystem, read root data and fat data
 */
void fat12_init()
{
  fat12_set_info(FAT12_INFO_READ);
  
  char* str = "mlegebazi";
  
  fat12_create("soga");
  fat12_write("soga", 0, str, strlen(str));
  print_dir();
}
