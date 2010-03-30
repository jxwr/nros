#pragma once

#define NULL 0

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef signed int s32;
typedef signed short s16;
typedef signed char s8;

typedef unsigned int size_t;
typedef int ssize_t;



static inline int isupper(int ch) {
  return (unsigned int)(ch - 'A') < 26u;
}

static inline int isalnum(int ch) {
  return (unsigned int)((ch | 0x20) - 'a') < 26u  ||
	 (unsigned int)( ch         - '0') < 10u;
}

static inline int isalpha(int ch) {
  return (unsigned int)((ch | 0x20) - 'a') < 26u;
}

static inline int isascii(int ch) 
{
  return (unsigned int)ch < 128u;
}

static inline int isdigit(int ch) {
  return (unsigned int)(ch - '0') < 10u;
}

static inline int isgraph(int ch) {
  return (unsigned int)(ch - '!') < 127u - '!';
}

static inline int islower(int ch) {
  return (unsigned int) (ch - 'a') < 26u;
}

static inline int isprint(int ch) {
  ch &= 0x7f;
  return (ch >= 32 && ch < 127);
}

static inline int isspace(int ch)
{
  return (unsigned int)(ch - 9) < 5u  ||  ch == ' ';
}

static inline int tolower(int ch) {
  if ((unsigned int)(ch - 'A') < 26u)
    ch += 'a' - 'A';
  return ch;
}

static inline int toupper(int ch) {
  if ((unsigned int)(ch - 'a') < 26u)
    ch += 'A' - 'a';
  return ch;
}
