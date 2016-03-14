// paging.c

#include "paging.h"
#include "lib.h"

// CONSTANTS
#define DEBUG 1
#define KERNEL_LOC 0x00400000


// FUNCTION DECLARATIONS
int paging_init(void);


// GLOBAL VARIABLES
static unsigned long pageDir[1024] __attribute__((aligned(4096)));
static unsigned long vidMemTable[1024] __attribute__((aligned(4096)));


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

    // initialize first table (video memory)
    if (DEBUG) {
        // sanity check
        if (vidMemTable & 0xFFFFF000) {
            printf("ERROR: vidMemTable not aligned to 4KB.\n");
            return -1;
        }
    }
    pageDir[0] = vidMemTable | 0x00000007; // sets flags to accessable-by-everyone, write-enabled, and present.
    for (i = 0; i < 1024; i++) {
        vidMemTable[i] = (i << 12) | 0x00000107; // maps video memory to 0x0 (and sets same flags as above plus global flag)
    }

    // initialize kernel
    pageDir[1] = KERNEL_LOC | 0x00000083; // maps kernel to 4MiB, sets flags to 4MiB-size, kernel-only, write-enabled, and present

    return 0;
}
