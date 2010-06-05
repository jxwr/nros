
int add(int a, int b)
{
  return a+b;
}


int main()
{
  int a,b,c;
  
  a = 10;
  b = 5;
  c = add(a, b);
  for(a = 0; a < 10; a++) {
    asm volatile("movl $4, %eax\n"
		 "int $0x40");
  }
  return 0;
}
