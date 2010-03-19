#include <stdio.h>

void cput_queue(unsigned int eax, unsigned int ebx, unsigned char mode)
{
  char a,b;
  a = (char)eax;
  b = (char)ebx;
  printf("(%c, %c)\t", a, b);
  printf("(%x,%x)\t", eax, ebx);
  printf("(mode %x)\n", mode);
}
