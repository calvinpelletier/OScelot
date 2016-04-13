// paging.h

#ifndef PAGING_H
#define PAGING_H


#include "types.h"

#define FOUR_MB   0x400000
#define PROGRAM_IMAGE 0x8000000

// GLOBAL VAR: pageDir

extern int32_t paging_init();
extern void loadPageDir(uint32_t *);
extern void enablePaging();
extern void enable4MB();
extern void new_page_directory(uint32_t PID);
extern void swap_pages(uint32_t PID);

#endif
