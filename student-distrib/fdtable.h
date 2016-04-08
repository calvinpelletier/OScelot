#ifndef FDTABLE_H
#define FDTABLE_H

#include "types.h"

#define MAX_FD 8

typedef struct {
    uint32_t in_use;
} flags_t;

typedef struct {
    uint32_t *file_ops_table_ptr;
    uint32_t inode;
    uint32_t position;
    flags_t flags;
} file_t;

void fd_init ();
int32_t get_fd ();
void remove_fd (int32_t);

#endif
