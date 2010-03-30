#pragma once
#include <list.h>
#include <ctype.h>

typedef struct inode_s {
  unsigned int i_count;
  unsigned int i_size;
  unsigned char i_mode;
  unsigned long i_mtime;
  unsigned long i_ctime;
  link_t i_list;
  void* i_data;
} inode_t;

struct file_s;
//typedef struct file_s file_t;

typedef struct file_operations_s {
  int (*open) (inode_t*, struct file_s*);
  ssize_t (*read) (struct file_s*, char*, size_t, unsigned int*);
  ssize_t (*write) (struct file_s*, char*, size_t, unsigned int*);
  int (*close) (inode_t*, struct file_s*);
} file_operations_t;

typedef struct file_s {
  link_t f_list;
  inode_t* f_inode;
  unsigned int f_pos;
  file_operations_t* f_ops;
} file_t;

typedef struct fs_operations_s {
  inode_t* (*get_inode)(const char* path);
  int (*put_inode)(inode_t* inode);
} fs_operations_t;

typedef struct file_system_s {
  char* fs_name;
  fs_operations_t* fs_ops;
} file_system_t;

extern file_system_t file_system;


/* system call, should move to other files */
int sys_open(const char* filename);
int sys_read(int fd, char* buf, size_t count);
int sys_write(int fd, char* buf, size_t count);
int sys_close(int fd);
