// paging.h

#ifndef PAGING_H
#define PAGING_H


#include "types.h"

#define FOUR_MB          0x00400000
#define USER_PAGE_BOTTOM 0x08400000
#define PROGRAM_IMAGE    0x08000000
#define VIDEO_MEMORY     0x000B8000

// GLOBAL VAR: pageDir

extern int32_t paging_init();
extern void loadPageDir(uint32_t *);
extern void enablePaging();
extern void enable4MB();
extern void new_page_directory(uint32_t PID);
extern void swap_pages(uint32_t PID);
extern int32_t new_page_directory_entry (uint32_t PID, uint32_t virt_addr, uint32_t phys_addr, uint8_t size, uint8_t privilege);
extern void hide_process(unsigned int PID);
extern void show_process(unsigned int PID);


#endif
