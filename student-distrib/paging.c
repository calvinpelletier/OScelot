// paging.c

#include "paging.h"
#include "lib.h"


// CONSTANTS
#define KERNEL_LOC 0x00400000


// FUNCTION DECLARATIONS
int32_t paging_init();

// GLOBAL VARIABLES
static uint32_t pageDir[7][1024] __attribute__((aligned(4096)));
static uint32_t first_4MB[7][1024] __attribute__((aligned(4096)));


/*
Page Directory Entry Format:
31   12     8 7 6 5 4 3 2 1 0
address ... - S - A C W U R P
S: page size (0 = 4KB, 1 = 4MB)
A: accessed (0 = no, 1 = yes)
C: cache (0 = enabled, 1 = disabled)
W: cache (0 = write-back, 1 = write-through)
U: user/kernel (0 = kernel, 1 = everyone)
R: read/write (0 = read-only, 1 = read/write)
P: present (0 = no, 1 = yes)
Page Table Entry Format:
31   12     8 7 6 5 4 3 2 1 0
address ... G - D A C W U R P
G: global (1 prevents TLB from updating address in cache)
D: dirty (0 = hasn't been written to, 1 = has been written to)
*/


// GLOBAL FUNCTIONS
/*
paging_init
    DESCRIPTION: initializes paging
    INPUTS: none
    OUTPUTS: none
    RETURNS: 0 for success, -1 for fail
*/
int32_t paging_init() {

    // initialize pageDir[0]
    int32_t i;
    for (i = 0; i < 1024; i++) {
        pageDir[0][i] = 0x00000002; // this sets the flags to kernel-only, write-enabled, and not-present
    }

    pageDir[0][0] = (uint32_t)(first_4MB[0]) | 0x00000003; // sets flags to accessible-by-kernel, write-enabled, and present
    for (i = 0; i < 1024; i++) {
        first_4MB[0][i] = (i * 0x1000) | 0x00000003; // sets flags to kernel, write-enabled, and present
    }
    first_4MB[0][0] &= ~0x00000001; // make first 4kB not present

    // initialize kernel 4 MB
    pageDir[0][1] = KERNEL_LOC | 0x00000083; // maps kernel to 4MiB, sets flags to 4MiB-size, kernel-only, write-enabled, and present

    // enable paging
    loadPageDir(pageDir[0]);
    enable4MB();
    enablePaging();

    return 0;
}

void new_page_directory(uint32_t PID) {
    // initialize pageDir[PID]
    int32_t i;
    for (i = 0; i < 1024; i++) {
        pageDir[PID][i] = 0x00000002; // this sets the flags to kernel-only, write-enabled, and not-present
    }

    pageDir[PID][0] = (uint32_t)(first_4MB[PID]) | 0x00000003; // sets flags to accessible-by-kernel, write-enabled, and present
    for (i = 0; i < 1024; i++) {
        first_4MB[PID][i] = (i * 0x1000) | 0x00000003; // sets flags to kernel, write-enabled, and present
    }
    first_4MB[PID][0] &= ~0x00000001; // make first 4kB not present

    // initialize kernel 4 MB
    pageDir[PID][1] = KERNEL_LOC | 0x00000083; // maps kernel to 4MiB, sets flags to 4MiB-size, kernel-only, write-enabled, and present

    uint32_t phys_addr = FOUR_MB * (PID + 1);
    uint32_t dir_entry = PROGRAM_IMAGE/ FOUR_MB;

    pageDir[PID][dir_entry] = phys_addr | 0x00000087; // 4MB page for program image is set to user, write-enabled, and present 

    // enable paging
    loadPageDir(pageDir[PID]);
    enable4MB();
    enablePaging();

}

void swap_pages(uint32_t PID) {
    loadPageDir(pageDir[PID]);
}
