#pragma once

/*
 * init FAT12 filesystem, read root data and fat data
 */
void fat12_init();

/*
 * create a new file in the root directory with size 0
 */
void fat12_create(char* filename);

/*
 * remove file
 */
void fat12_remove(char* filename);

/* 
 * read 'size bytes of the file in the root directory from 'offset into 'buffer,
 * if filesize is less than 'offset + 'size, filesize will change to that large
 */
void fat12_read(char* filename, int offset, void* buffer, size_t size);

/* 
 * write 'size bytes of the 'buffer in the file
 */
void fat12_write(char* filename, int offset, void* buffer, size_t size);
