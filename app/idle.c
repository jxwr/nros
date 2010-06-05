

int main()
{
  while(1) {
    asm volatile("movl $2, %eax\n"
		 "int $0x40");
  }

  return 0;
}
