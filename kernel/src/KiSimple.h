#ifndef KISIMPLE_H
#define KISIMPLE_H 1

#include <flanterm/flanterm.h>
#include <flanterm/flanterm_backends/fb.h>
#include <printk/printk.h>

typedef void* ptr;

void outb(uint16_t port, uint8_t byte);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t word);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t dword);
uint32_t inl(uint16_t port);
void IOWait();
void insw(uint16_t port, void* addr, int count);
void outsw(uint16_t port, const void* addr, int count);

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
char* strrchr(const char* s, int c);

void KiPanic(const char* __restrict string, int _halt);
void DisplaySplash(int w, int h, char* text); /* w: width of display in characters, h: height of display in characters */

#endif /* KISIMPLE_H */
