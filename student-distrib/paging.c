// paging.c

#include "paging.h"
#include "lib.h"


// FUNCTION DECLARATIONS
void paging_init(void);


// GLOBAL VARIABLES
static unsigned int pageDir[1024] __attribute__((aligned(4096)));


// GLOBAL FUNCTIONS
void paging_init(void) {
    // initialize pageDir
    int i;
    for (i = 0; i < 1024; i++) {
        pageDir[i] = 0x00000002; // this sets the flags to kernel-only, write-enabled, and not-present
    }
}
