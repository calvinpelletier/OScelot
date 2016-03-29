// filesys.h

#ifndef FILESYS_H
#define FILESYS_H

#include "types.h"

// CONSTANTS
#define MAX_FNAME_LEN 32
#define DENTRY_RESERVED 24
#define BOOTBLOCK_RESERVED 52
#define MAX_DENTRIES 63
#define DATABLOCKS_PER_INODE 1023
#define FILEARRAY_SIZE 8
#define FS_BLOCK_SIZE 4096

// STRUCTS
typedef struct {
    char name[MAX_FNAME_LEN];
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
    int (*open)(const char*);
    int (*read)(int, unsigned char*, int);
    int (*write)(int, unsigned char*, int);
    int (*close)(int);
} fileops_t;

typedef struct {
    unsigned int in_use : 1; // occupies 1 bit (total struct size 4 bytes)
    unsigned int read_only : 1;
    unsigned int write_only : 1;
} fileflags_t;

typedef struct {
    fileops_t jumptable;
    unsigned int inode;
    unsigned int position;
    unsigned int filetype;
    fileflags_t flags;
} file_t;


// GLOBAL FUNCTIONS
extern int fs_init(void* start, void* end);
extern int test_demo1(char* filename);
extern int test_demo2(char* filename);
extern int test_demo3(void);


#endif
