/*
 * must put this before any code
 */
asm (".code16gcc;");

#include <nros/port.h>
#include <nros/bootinfo.h>

extern char gdtr;
extern char idtr;
extern char pmode_entry;
void die();

boot_info_t boot_info;

/*
 * we must rewrite this here for 16-bit gcc 
 */
static void* memcpy(void* to, void* from, size_t n)
{
  register char* t = to;
  register const char* f = from;
  
  while(n--)
    *t++ = *f++;
  return to;
}

/*
 * display functions
 */
static void putchar(char ch)
{
  unsigned char c = ch;
  if(c == '\n')
    putchar('\r');
  asm volatile("pushal;"
	       "pushw %%ds;"
	       "int $0x10;"
	       "popw %%ds;"
	       "popal"
	       :: 
		"b" (0x0007), 
		"c" (0x0001),
		"a" (0x0e00|ch));
}

static void puts(char* str)
{
  char* s = str;
  while(*s)
    putchar(*s++);
}

static void put_hex(unsigned int n)
{
  int i;
  puts("0x");
  for(i = 7; i >= 0; i--) {
    char t = (n >> i*4) & 0x0f;
    if(t < 10)
      putchar(t + '0');
    else
      putchar(t + 'A' - 10);
  }
}

/*
 * remap 8259 Programmable Interrupt Controller
 */
static void remap_pic()
{
  outb(0x11,0x20);
  io_delay();
  outb(0x11,0xa0);
  io_delay();
  outb(0x20,0x21);
  io_delay();
  outb(0x28,0xa1);
  io_delay();
  outb(0x04,0x21);
  io_delay();
  outb(0x02,0xa1);
  io_delay();
  outb(0x01,0x21);
  io_delay();
  outb(0x01,0xa1);
  io_delay();
  outb(0xff,0x21);
  io_delay();
  outb(0xff,0xa1);
}

static void set_gdtr()
{
  asm volatile("lgdtl %0;" : : "m" (gdtr));
}

static void set_idtr()
{
  asm volatile("lidtl %0;" : : "m" (idtr));
}

static void kill_all_irqs()
{
  outb(0xff,0xa1);
  outb(0xff,0x21);
}

static void goto_pmode()
{
  asm volatile("movl %cr0,%eax;"
	       "orl $1,%eax;"
	       "movl %eax,%cr0;"
	       "ljmpl $0x08, $pmode_entry;"
	       );
}

static void open_a20()
{
  u16 port = 0x92;
  u8 val = inb(port);
  val |= 2;
  outb(val, port);
}

static void cli()
{
  asm volatile("cli;");
}



/*
 * use e801, because it is easy
 */
static void detect_memory_size(void)
{
  u16 ax = 0xe801;
  u16 bx = 0;
  u8 err;
  asm volatile("stc; int $0x15; setc %0"
      : "=m" (err), "+a" (ax), "+b" (bx));
  if(err || ax > 15*1024){
    puts("detect memory error\n");
    die();
  }
  
  boot_info.mem_size = (ax == 15*1024) ? (bx << 6)+ax : ax;
  boot_info.mem_size += 1024;
}

void save_boot_info()
{
  void* dst = (void*)0x9000;
  void* src = &boot_info;
  memcpy(dst, src, sizeof(boot_info));
}


int main()
{
  cli();
  kill_all_irqs();

  puts("open a20 address line ...\n");
  open_a20();

  puts("remaping interrupt ...\n");
  remap_pic();

  puts("detect memory ...\n");
  detect_memory_size();

  puts("mem size: ");
  put_hex(boot_info.mem_size);
  putchar('\n');
  save_boot_info();

  puts("setup gdt and idt ...\n");
  set_gdtr();
  set_idtr();

  puts("enter pmode...\n");
  
  /*
   * set cursor to the bottom of screen
   */
  {
    int i;
    for(i = 0; i < 25; i++)
      putchar('\n');
  }
  goto_pmode();

  return 1;
}
