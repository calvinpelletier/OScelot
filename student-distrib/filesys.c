// filesys.c

#include "filesys.h"


// STRUCTS
struct dentry_t {
    unsigned char name[32];
    int type;
    unsigned int inode;
    unsigned char reserved[24];
};

struct bootblock_t {
    unsigned int n_dentries;
    unsigned int n_inodes;
    unsigned int n_datablocks;
    unsigned char reserved[52];
    dentry_t dentries[63];
};

struct inode_t {
    unsigned int length; // in bytes
    unsigned int datablocks[1023];
};

struct fileops_t {
    int (*open)(unsigned char*);
    int (*read)(int, unsigned char*, int);
    int (*write)(int, unsigned char*, int);
    void (*close)(int);
};

struct fileflags_t {
    unsigned int in_use : 1;
    unsigned int read_only : 1;
    unsigned int write_only : 1;
}

struct file_t {
    fileops_t jumptable;
    inode_t inode;
    unsigned int position;
    fileflags_t flags;
}


// GLOBAL VARIABLES



// FUNCTION DECLARATIONS
int init_filesys(void);


// GLOBAL FUNCTIONS
int init_filesys(void) {

}
