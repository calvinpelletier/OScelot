// filesys.c

#include "filesys.h"


// FUNCTION DECLARATIONS
int init_filesys(void);
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry);
int read_dentry_by_index(unsigned int index, dentry_t* dentry);
int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length);

// GLOBAL FUNCTIONS
int init_filesys(void) {
    
}
