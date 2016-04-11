// paging.h

#ifndef PAGING_H
#define PAGING_H


#include "types.h"

#define FOUR_MB   4194304
#define VIRT_ADDR 134217728

// GLOBAL VAR: pageDir
extern int paging_init(void);
extern void loadPageDir(unsigned long *);
extern void enablePaging(void);
extern void enable4MB(void);
extern void new_page(unsigned int physical, unsigned int process_ID);

#endif
