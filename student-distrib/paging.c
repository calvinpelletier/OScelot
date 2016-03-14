// paging.c

#include "paging.h"
#include "lib.h"


// CONSTANTS
#define DEBUG 1
#define KERNEL_LOC 0x00400000


// FUNCTION DECLARATIONS
int paging_init(void);
void * virt_to_phys(void *);


// GLOBAL VARIABLES
static unsigned long pageDir[1024] __attribute__((aligned(4096)));
static unsigned long first_4MB[1024] __attribute__((aligned(4096)));


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
int paging_init(void) {
    // initialize pageDir
    int i;
    for (i = 0; i < 1024; i++) {
        pageDir[i] = 0x00000002; // this sets the flags to kernel-only, write-enabled, and not-present
    }

    // initialize first table
    if (DEBUG) {
        // sanity check
        if ((unsigned long)(first_4MB) & 0x00000FFF) {
            printf("ERROR: first_4MB table not aligned to 4KB.\n");
            printf("Address: %x\n", (unsigned long)(first_4MB));
            return -1;
        }
    }

    
    pageDir[0] = (unsigned long)(first_4MB) | 0x00000003; // sets flags to accessable-by-kernel, write-enabled, and present.
    for (i = 0; i < 1024; i++) {
        first_4MB[i] = (i * 0x1000) | 0x00000003; // sets flags to kernel, write-enabled, and present
    }
    first_4MB[0] &= ~0x00000003; // make first 4kB not present and not writable

    // initialize kernel 4 MB
    pageDir[1] = KERNEL_LOC | 0x00000083; // maps kernel to 4MiB, sets flags to 4MiB-size, kernel-only, write-enabled, and present

    // enable paging
    loadPageDir(pageDir);
    enable4MB();
    enablePaging();

    return 0;
}

/*
void * virt_to_phys(void * addr) {
    int pageDirIdx = (int)(addr >> 22);
    int pageTblIdx = (int)((addr >> 12) & 0x000003FF);
    if ()
}*/
