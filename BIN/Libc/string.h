#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, unsigned int n);
unsigned int strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, unsigned int n);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, unsigned int n);
char* strchr(const char* str, int c);
char* strstr(const char* haystack, const char* needle);
char* strtok(char* str, const char* delim);

#endif
