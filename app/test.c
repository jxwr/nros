
int main()
{
  int a,b,c;
  
  a = 10;
  b = 5;
  c = a + b;
  for(a = 0; a < 10000; a++)
    asm volatile("int $0x40");
  return 0;
}
