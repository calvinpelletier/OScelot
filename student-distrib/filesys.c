// filesys.c

#include "filesys.h"
#include "lib.h"

// CONSTANTS
#define MAX_FNAME_LEN 32
#define DENTRY_RESERVED 24
#define BOOTBLOCK_RESERVED 52
#define MAX_DENTRIES 63
#define DATABLOCKS_PER_INODE 1023
#define FILEARRAY_SIZE 8

// STRUCTS
typedef struct {
    unsigned char name[MAX_FNAME_LEN];
    int type;
    unsigned int inode;
    unsigned char reserved[DENTRY_RESERVED];
} dentry_t;

typedef struct {
    unsigned int n_dentries;
    unsigned int n_inodes;
    unsigned int n_datablocks;
    unsigned char reserved[BOOTBLOCK_RESERVED];
    dentry_t dentries[MAX_DENTRIES];
} bootblock_t;

typedef struct {
    unsigned int length; // in bytes
    unsigned int datablocks[DATABLOCKS_PER_INODE];
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
static file_t filearray[FILEARRAY_SIZE]; // 0 = stdin, 1 = stdout, 2-7 = free to use
static bootblock_t bootblock;


// FUNCTION DECLARATIONS
int filesys_init(void* start, void* end);
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry);
int read_dentry_by_index(unsigned int index, dentry_t* dentry);
int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length);
int test(void);


// EXTERNAL FUNCTIONS
int filesys_init(void* start, void* end) {
    FILESYS_START = start;
    FILESYS_END = end;

    // populate bootblock
    bootblock = *((bootblock_t*)start);

    if (DEBUG_ALL) {
        test();
    }

    return 0;
}


// HELPER FUNCTIONS
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry) {
    int len = 0;
    while (len <= MAX_FNAME_LEN && fname[len]) {
        len++;
    }

    int i = 0;
    while (i < bootblock.n_dentries && i < MAX_DENTRIES) {
        printf("check0\n");
        if (!strncmp(fname, bootblock.dentries[i].name, len)) {
            printf("check1\n");
            // found match
            *dentry = bootblock.dentries[i];
            printf("check2\n");
            return 0;
        }
    }

    return -1;
}

int read_dentry_by_index(unsigned int index, dentry_t* dentry) {
    return 0;
}

int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length) {
    return 0;
}


// TESTING FUNCTIONS
int test(void) {
    int ret = 0;
    printf("~~~FILE SYSTEM TEST~~~\n");
    printf("n_dentries: %d\n", bootblock.n_dentries);
    printf("n_inodes: %d\n", bootblock.n_inodes);
    printf("n_datablocks: %d\n", bootblock.n_datablocks);
    dentry_t temp;
    int result = read_dentry_by_name(".", &temp);
    if (result) {
        printf("FAIL: did not find '.' directory entry\n");
        ret = -1;
    } else {
        printf("dentry name: %s, type: %d, inode: %d\n", temp.name, temp.type, temp.inode);
    }
    printf("~~~~~~\n");
    return ret;
}













// asdf
