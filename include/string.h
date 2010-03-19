#pragma once
#include <ctype.h>

void* memset(void* mem, int val, size_t count);
void* memcpy(void* to, const void* from, size_t count);
void* memchr(const void* str, int c, size_t count);
void* memmove(void* to, const void* from, size_t count);

size_t strlen(const char* str);
char* strcpy(char* to, const char* from);
char* strncpy(char* to, const char* from, size_t count);
char* strcat(char* str1, const char* str2);
char* strncat(char* str1, const char* str2, size_t count);
char* strchr(const char* str, int c);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
