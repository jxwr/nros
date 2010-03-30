#include <nros/fs.h>
#include <nros/proc.h>

file_operations_t fat12_fops;

int sys_open(const char* filename)
{
  inode_t* inode = file_system.fs_ops->get_inode(filename);
  int fd = get_free_fd(current);
  file_t* filp;

  current->filp[fd] = kmalloc(sizeof(file_t));
  filp = current->filp[fd];

  filp->f_ops = &fat12_fops;

  if(fd >= 0) {
    filp->f_ops->open(inode, filp);
    return fd;
  }

  return -1;
}

int sys_read(int fd, char* buf, size_t count)
{
  file_t* filp = current->filp[fd];

  int res = filp->f_ops->read(filp, buf, count, &filp->f_pos);

  return res;
}

int sys_write(int fd, char* buf, size_t count)
{
  file_t* filp = current->filp[fd];

  int res = filp->f_ops->write(filp, buf, count, &filp->f_pos);

  return res;
}

int sys_close(int fd)
{
  file_t* filp = current->filp[fd];
  inode_t* inode = filp->f_inode;

  int res = filp->f_ops->close(inode, filp);
  file_system.fs_ops->put_inode(inode);
  kfree(filp);
  current->filp[fd] = NULL;

  return res;
}
