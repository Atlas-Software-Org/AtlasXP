#ifndef STDIO_H
#define STDIO_H

#include "asnulibc.h"

int printf(const char* fmt, ...);
int fputs(const char* s, FILE* f);
int fputc(int c, FILE* f);
int fgetc(FILE* f);
char* fgets(char* buf, int size, FILE* f);
void puts(const char* s);
void putchar(char c);

void termgetres(unsigned long long* width, unsigned long long* height);

#endif
