// paging.h

#ifndef PAGING_H
#define PAGING_H


#include "types.h"

#define FOUR_MB   0x400000
#define PROGRAM_IMAGE 0x8000000

// GLOBAL VAR: pageDir
extern int paging_init(void);
extern void loadPageDir(unsigned long *);
extern void enablePaging(void);
extern void enable4MB(void);
extern void new_page_directory(unsigned int PID);

#endif
