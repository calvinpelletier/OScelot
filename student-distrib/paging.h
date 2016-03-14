// paging.h

#ifndef PAGING_H
#define PAGING_H


#include "types.h"

// GLOBAL VAR: pageDir
extern int paging_init(void);
extern void loadPageDir(unsigned long *);
extern void enablePaging(void);
//extern void * virt_to_phys(void *);

#endif