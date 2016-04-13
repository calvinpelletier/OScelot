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
#define FS_BLOCK_SIZE 4096

// STRUCTS
typedef struct {
    int8_t  name[MAX_FNAME_LEN];
    int32_t type;
    uint32_t inode;
    uint8_t  reserved[DENTRY_RESERVED];
} dentry_t;

typedef struct {
    uint32_t n_dentries;
    uint32_t n_inodes;
    uint32_t n_datablocks;
    uint8_t  reserved[BOOTBLOCK_RESERVED];
    dentry_t dentries[MAX_DENTRIES];
} bootblock_t;

typedef struct {
    uint32_t length; // in bytes
    uint32_t datablocks[DATABLOCKS_PER_INODE];
} inode_t;

typedef struct {
    uint32_t in_use : 1; // occupies 1 bit (total struct size 4 bytes)
    uint32_t read_only : 1;
    uint32_t write_only : 1;
} fileflags_t;

struct file;
struct fileops;
typedef struct file file_t ;
typedef struct fileops fileops_t;

struct file {
    fileops_t * jumptable;
    uint32_t inode;
    uint32_t position;
    uint32_t filetype;
    fileflags_t flags;
};

struct fileops {
    int32_t (*open)();
    int32_t (*read)(file_t*, uint8_t *, int32_t);
    int32_t (*write)(file_t*, uint8_t *, int32_t);
    int32_t (*close)(file_t*);
};


// GLOBAL FUNCTIONS
extern int32_t fs_init(void* start, void* end);
extern int32_t fs_copy(const int8_t * fname, uint8_t * mem_location);
extern int32_t fs_open ();
extern int32_t fs_close(file_t* file);
extern int32_t fs_read (file_t* file, uint8_t * buf, int32_t nbytes);
extern int32_t fs_write (file_t* file, uint8_t * buf, int32_t nbytes);
extern int32_t read_dentry_by_name(const int8_t * fname, dentry_t* dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
extern int32_t test_demo1(int8_t * filename);
extern int32_t test_demo2(int8_t * filename);
extern int32_t test_demo3();


#endif
