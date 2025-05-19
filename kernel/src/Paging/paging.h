#ifndef PAGING_H
#define PAGING_H 1

#include <stdint.h>

typedef uint64_t pte_t;

pte_t* KiPml4Init();

#endif /* PAGING_H */