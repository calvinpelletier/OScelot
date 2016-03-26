// filesys.c

#include "filesys.h"


// STRUCTS
typedef struct {
    unsigned char name[32];
    int type;
    unsigned int inode;
    unsigned char reserved[24];
} dentry_t;

typedef struct {
    unsigned int n_dentries;
    unsigned int n_inodes;
    unsigned int n_datablocks;
    unsigned char reserved[52];
    dentry_t dentries[63];
} bootblock_t;

typedef struct {
    unsigned int length; // in bytes
    unsigned int datablocks[1023];
} inode_t;

typedef struct {
    int (*open)(unsigned char*);
    int (*read)(int, unsigned char*, int);
    int (*write)(int, unsigned char*, int);
    void (*close)(int);
} fileops_t;

typedef struct {
    unsigned int in_use : 1;
    unsigned int read_only : 1;
    unsigned int write_only : 1;
} fileflags_t;

typedef struct {
    fileops_t jumptable;
    inode_t inode;
    unsigned int position;
    fileflags_t flags;
} file_t;


// GLOBAL VARIABLES
// each process will have its own filearray when we implement that
static void* FILESYS_START;
static void* FILESYS_END;
static file_t filearray[8]; // 0 = stdin, 1 = stdout, 2-7 = free to use
static bootblock_t bootblock;


// FUNCTION DECLARATIONS
int filesys_init(void* start, void* end);
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry);
int read_dentry_by_index(unsigned int index, dentry_t* dentry);
int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length);


// EXTERNAL FUNCTIONS
int filesys_init(void* start, void* end) {
    FILESYS_START = start;
    FILESYS_END = end;
    return 0;
}


// HELPER FUNCTIONS
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry) {
    return 0;
}
int read_dentry_by_index(unsigned int index, dentry_t* dentry {
    return 0;
}
int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length) {
    return 0;
}
