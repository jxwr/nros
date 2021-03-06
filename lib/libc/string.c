#include <ctype.h>
#include <nros/macro.h>


void* memset(void* mem, int val, size_t count)
{
  register unsigned char* p = mem;

  while(count--) {
    *p++ = (unsigned char)val;
  }

  return mem;
}

void* memcpy(void* to, const void* from, size_t count)
{
  register unsigned char* t = to;
  register const unsigned char* f = from;

  while(count--) {
    *t++ = *f++;
  }

  return to;
}

int memcmp(const void *mem1, const void *mem2, size_t count) 
{
  register int res;
  register const unsigned char *m1 = mem1;
  register const unsigned char *m2 = mem2;

  ++count;

  while (likely(--count)) {
    if (unlikely(res = (*m1 - *m2)))
      return res;
    ++m1;
    ++m2;
  }

  return 0;
}

void* memchr(const void* str, int c, size_t count) 
{
  register const unsigned char *s = (unsigned char *) str;

  for (; count--; s++)
    if (*s == c)
      return ((void *)s);

  return 0;
}

void* memmove(void* to, const void* from, size_t count)
{
  register unsigned char* t = t;
  register const unsigned char* f = from;

  if(f >= t) {
    while(count) {
      *t++ = *f++;
      --count;
    }
  }
  else {
    while(count) {
      --count;
      t[count] = f[count];
    }
  }

  return to;
}

/* Magic numbers for the algorithm */
static const unsigned long mask01 = 0x01010101;
static const unsigned long mask80 = 0x80808080;

#define	LONGPTR_MASK (sizeof(long) - 1)

/*
 * Helper macro to return string length if we caught the zero
 * byte.
 */
#define testbyte(x)				\
	do {					\
		if (p[x] == '\0')		\
		    return (p - str + x);	\
	} while (0)

size_t strlen(const char* str)
{
  const char* p;
  const unsigned long* lp;

  /* Skip the first few bytes until we have an aligned p */
  for (p = str; (unsigned long)p & LONGPTR_MASK; p++)
    if (*p == '\0')
      return (p - str);

  /* Scan the rest of the string using word sized operation */
  for (lp = (const unsigned long *)p; ; lp++)
    if ((*lp - mask01) & mask80) {
      p = (const char *)(lp);
      testbyte(0);
      testbyte(1);
      testbyte(2);
      testbyte(3);
    }

  /* NOTREACHED */
  return 0;
}

char* strcpy(char* to, const char* from)
{
  register char* t = to;
  register const char* f = from;

  while((*t++ = *f++) != 0);

  return to;
}

char* strncpy(char* to, const char* from, size_t count)
{
  register char* t = to;
  register const char* f = from;

  while(count--){
    if(*f != 0)
      *t++ = *f++;
    else
      break;
  }

  return to;
}

char* strcat(char* str1, const char* str2)
{
  register char* s1 = str1;
  register const char* s2 = str2;

  s1 += strlen(s1);

  for (;;) {
    if (!(*s1 = *s2)) break; ++s1; ++s2;
    if (!(*s1 = *s2)) break; ++s1; ++s2;
    if (!(*s1 = *s2)) break; ++s1; ++s2;
    if (!(*s1 = *s2)) break; ++s1; ++s2;
  }

  return str1;
}

char* strncat(char* str1, const char* str2, size_t count) 
{
  register char* s1 = str1;
  register const char* s2 = str2;
  register char *max;

  s1 += strlen(s1);

  if (unlikely((max = s1 + count) == s1)) 
    goto out;

  for (;;) {
    if (unlikely(!(*s1 = *s2))) break; if (unlikely(++s1 == max)) break; ++s2;
    if (unlikely(!(*s1 = *s2))) break; if (unlikely(++s1 == max)) break; ++s2;
    if (unlikely(!(*s1 = *s2))) break; if (unlikely(++s1 == max)) break; ++s2;
    if (unlikely(!(*s1 = *s2))) break; if (unlikely(++s1 == max)) break; ++s2;
  }

  *s1=0;

out:
  return str1;
}

int strcmp(const char* str1, const char* str2)
{
  register const char* s1 = str1;
  register const char* s2 = str2;
  
  while (*s1 && *s1 == *s2)
    s1++, s2++;
  return (*s1 - *s2);
}

int strncmp(const char* str1, const char* str2, size_t n) 
{
  register const char* s1 = str1;
  register const char* s2 = str2;
  register const char* end = s1 + n;

  while (s1 < end) {
    register int res = *s1 - *s2;
    if (res) return res;
    if (!*s1) return 0;
    ++s1; ++s2;
  }

  return 0;
}

char* strchr(const char* str, int c) 
{
  register const char* s;
  register char ch;

  s = str;
  ch = c;

  for (;;) {
    if (unlikely(*s == ch)) break; if (unlikely(!*s)) return 0; ++s;
    if (unlikely(*s == ch)) break; if (unlikely(!*s)) return 0; ++s;
    if (unlikely(*s == ch)) break; if (unlikely(!*s)) return 0; ++s;
    if (unlikely(*s == ch)) break; if (unlikely(!*s)) return 0; ++s;
  }

  return (char*)s;
}
