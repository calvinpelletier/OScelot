// paging.c

#include "paging.h"
#include "lib.h"


// FUNCTION DECLARATIONS
void paging_init(void);


// GLOBAL VARIABLES
static unsigned int pageDir[1024] __attribute__((aligned(4096)));
static unsigned int vidMemTable[1024] __attribute__((aligned(4096)));


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
void paging_init(void) {
    // initialize pageDir
    int i;
    for (i = 0; i < 1024; i++) {
        pageDir[i] = 0x00000002; // this sets the flags to kernel-only, write-enabled, and not-present
    }

    // initialize first table (video memory)

    // initialize kernel

}
