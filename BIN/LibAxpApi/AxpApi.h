#ifndef AXPAPI_H
#define AXPAPI_H 1

#include <stdint.h>

int write(int fd, void* buf, int count);
int read(int fd, void* buf, int count);
int getpid();
void exit(int code);
void mmap(void* virt, void* phys, uint64_t attr);
void umap(void* virt);
void* alloc();
void free(void* ptr);

#endif /* AXPAPI_H */
