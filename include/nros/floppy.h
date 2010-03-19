#pragma once

#define FD_BLOCK_SIZE 1024
#define FD_SECTOR_SIZE 512

#define DISK_READ   0
#define DISK_WRITE  1

extern char dma_buffer[1024];

void fd_init();

void fd_rw(int cmd, unsigned long sector_linear);


