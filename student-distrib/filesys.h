// filesys.h

#ifndef FILESYS_H
#define FILESYS_H

#include "types.h"

struct dentry_t {
    unsigned char name[32];
    int type;
    unsigned int n_inodes;
    unsigned char reserved[24];
}


// GLOBAL FUNCTIONS


#endif
