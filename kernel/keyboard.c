#include <stdio.h>
#include <nros/tty.h>

#define _put(ptr, n)				\
  do{						\
    if(ptr[n] != 0) {				\
      PUTCH(ptr[n], tty.write_q);		\
      if(ptr[n] == 13)				\
	PUTCH('\r', tty.write_q);		\
    }						\
  } while(0)

void cput_queue(unsigned int eax, unsigned int ebx, unsigned char mode)
{
  const char* p = (char*)&eax;
  const char* q = (char*)&ebx;
  
  _put(p, 0);
  _put(p, 1);
  _put(p, 2);
  _put(p, 3);
  _put(q, 0);
  _put(q, 1);
  _put(q, 2);
  _put(q, 3);

  con_write(&tty);

  //  printf("(%x,%x)\t", eax, ebx);
  //  printf("(mode %x)\n", mode);
}
