#include <nros/macro.h>
#include <ctype.h>
#include <stdarg.h>

#define GVM_BASE	(KVM_BASE + 0xA0000) // Graphics Video Memory
#define MTVM_BASE	(KVM_BASE + 0xB0000) // Momochrome Text Video Memory
#define CTVM_BASE	(KVM_BASE + 0xB8000) // Color Text Video Memory

#define LINE_WORDS 80
#define LINE_COUNT 25

static int cur_pos = 0;
static u16 text_color = 0x0700;

void set_pos(int pos)
{
  cur_pos = pos;
}

static void scroll_down()
{
  int i;
  u16* screen = (u16*)CTVM_BASE;
  for(i = LINE_WORDS; i < LINE_WORDS*LINE_COUNT; i++) {
    screen[i-80] = screen[i];
  }
  for(i = 0; i < LINE_WORDS; i++) {
    int j = i + LINE_WORDS*(LINE_COUNT-1);
    screen[j] = text_color|' ';
  }
  cur_pos -= 80;
}

void put_char(int ch)
{
  u16* screen = (u16*)CTVM_BASE;
  u16 val = text_color|ch;
  int i;
  switch(ch) {
  case '\r':
  case '\n':
    for(i = cur_pos%80;i < 80; i++) {
      put_char(' ');
    }
    return;
  case '\t':
    if(cur_pos/LINE_WORDS == (cur_pos+8)/LINE_WORDS)
      for(i = 0; i < 8; i++)
	put_char(' ');
    return;
  default:
    if(cur_pos/80 >= 25) {
      scroll_down();
    }
    screen[cur_pos++] = val;
  }
}

void puts(const char* str)
{
  while(*str)
      put_char(*str++);
}


static void put_hex(int n)
{
  int i, val;
  puts("0x");
  for(i = 7; i >= 0; i--) {
    val = (n >> i*4) & 0x0f;
    if(val < 10)
      put_char(val + '0');
    else
      put_char(val + 'a' - 10);
  }
}

static void put_int(int n)
{

  char buf[10];
  int i;
  if(n == 0) {
    put_char('0');
    return;
  }
  if(n < 0) {
    put_char('-');
    n = -n;
  }
  for(i = 0; i < 10; i++) {
    buf[i] = n % 10 + '0';
    n = n / 10;
  }
  while(buf[--i] == '0');
  while(i >= 0)
    put_char(buf[i--]);
}

void printf(const char* fmt, ...)
{
  register const char* s = fmt;
  va_list args;
  va_start(args,fmt);

  while(*s) {
    if(*s == '%' && ++s) {
      switch(*s++) {
      case 'c':
	put_char(va_arg(args,int));
	break;
      case 's':
	puts(va_arg(args,char*));
	break;
      case 'd':
	put_int(va_arg(args,int));
	break;
      case 'x':
	put_hex(va_arg(args,int));
	break;
      default:
	break;
      }
    }
    else
      put_char(*s++);
  }
  va_end(args);
}

void clear_screen()
{
  int i;
  set_pos(0);
  for(i = 0;i < LINE_COUNT; i++)
    put_char('\n');
  set_pos(0);
}

void panic(char* fmt, ...) {
  const char* s = fmt;
  va_list args;
  va_start(args,fmt);
  printf("PANIC: ");

  while(*s) {
    if(*s == '%' && ++s) {
      switch(*s++) {
      case 'c':
	put_char(va_arg(args,int));
	break;
      case 's':
	puts(va_arg(args,char*));
	break;
      case 'd':
	put_int(va_arg(args,int));
	break;
      case 'x':
	put_hex(va_arg(args,int));
	break;
      default:
	break;
      }
    }
    else
      put_char(*s++);
  }
  va_end(args);
  for(;;);
}
