// syscalls.c

#include "syscalls.h"
#include "lib.h"
#include "filesys.h"

// CONSTANTS
#define MAX_FNAME_LEN 32;


// FUNCTION DECLARATIONS
int halt (unsigned char status);
int execute (unsigned char* command);
int read (int fd, void* buf, int nbytes);
int write (int fd, const void* buf, int nbytes);
int open (const unsigned char* filename);
int close (int fd);
int getargs (unsigned char* buf, int nbytes);
int vidmap (unsigned char** screenstart);
int set_handler (int signum, void* handler_address);
int sigreturn (void);


int halt (unsigned char status) {
    return -1;
}

int execute (unsigned char* command) {
    unsigned char exename[MAX_FNAME_LEN];
    int i;

    // parse
    if (command == NULL) {
        return -1;
    }
    for (i = 0; command[i] != '\0' && command[i] != ' '; i++) {
        exename[i] = command[i];
    }

    // exe check
    unsigned char buf[MAX_FNAME_LEN];
    int fd, count;
    if ((fd = fs_open((char*)".")) == -1) {
        return -1;
    }
    int fail = 1;
    while ((count = fs_read(fd, buf, MAX_FNAME_LEN))) {
        if (count == -1) {
            return -1;
        }
        if (!strncmp(exename, buf, MAX_FNAME_LEN)) {
            fail = 0;
            break;
        }
    }
    if (fail) {
        return -1;
    }

    // set up paging
    // file loader
    // new pcb
    // write tss esp0 ebp0 with new kernel stack
    // save current esp ebp or anything you need in pcb
    // push artificial iret context onto stack
    // iret
    // halt_ret_label
    // ret
}

int read (int fd, void *buf, int nbytes) {
    int out = pcb[fd].file_ops_table_ptr[READ];
    return out;
}

int write (int fd, const void *buf, int nbytes) {
    int out = pcb[fd].file_ops_table_ptr[WRITE];
    return out;
}

int open (const unsigned char *filename) {
    return -1;
}

int close (int fd) {
    return -1;
}

int getargs (unsigned char *buf, int nbytes) {
    return -1;
}

int vidmap (unsigned char **screenstart) {
    return -1;
}

int set_handler (int signum, void *handler_address) {
    return -1;
}

int sigreturn (void) {
    return -1;
}
