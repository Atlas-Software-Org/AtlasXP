#ifndef ASNUAPI_H
#define ASNUAPI_H 1

#include <stdint.h>

int write(int fd, void* buf, int count);
int read(int fd, void* buf, int count);
int asnu_api_getpid();
void asnu_api_exit(int code);
void mmap(void* virt, void* phys, uint64_t attr);
void umap(void* virt);
void* memalloc();
void memfree(void* ptr);
uint64_t api_get_term_width();
uint64_t api_get_term_height();

#endif /* ASNUAPI_H */
