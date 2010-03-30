#pragma once
#include <nros/fs.h>

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

