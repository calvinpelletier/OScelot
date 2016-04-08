#include "fdtable.h"

static file_t pcb[MAX_FD];

void fd_init()
{
    int tmp;
    for (tmp = 0; tmp < MAX_FD; tmp++) {
        if (tmp < 2) {
            pcb[tmp].flags.in_use = 1;
        } else {
            pcb[tmp].flags.in_use = 0;
        }
    }
}

uint32_t get_fd()
{
}
