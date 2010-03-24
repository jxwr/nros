
int main()
{
  int a,b,c;
  
  a = 10;
  b = 5;
  c = a + b;
   
  for(;;) {
    asm volatile("int $0x40");
  }
}
