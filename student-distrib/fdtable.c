#include "fdtable.h"

static file_t pcb[MAX_FD];

void fd_init ()
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

int32_t get_fd ()
{
    int tmp;
    for (tmp = 0; tmp < MAX_FD; tmp++) {
        if (pcb[tmp].flags.in_use == 0) {
            pcb[tmp].flags.in_use = 1;
            return tmp;
        }
        return -1;
    }
}

void remove_fd (int32_t fd)
{
    pcb[fd].flags.in_use = 0;
    return;
}
