#ifndef STDLIB_H
#define STDLIB_H

void* malloc(int size);
void* calloc(int n, int s);
void free(void* ptr);
void exit(int code);
int getpid();

#endif
