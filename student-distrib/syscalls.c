// syscalls.c

#include "syscalls.h"
#include "lib.h"

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
    // parse
    // exe check
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

int read (int fd, void* buf, int nbytes) {
    return -1;
}

int write (int fd, const void* buf, int nbytes) {
    return -1;
}

int open (const unsigned char* filename) {
    return -1;
}

int close (int fd) {
    return -1;
}

int getargs (unsigned char* buf, int nbytes) {
    return -1;
}

int vidmap (unsigned char** screenstart) {
    return -1;
}

int set_handler (int signum, void* handler_address) {
    return -1;
}

int sigreturn (void) {
    return -1;
}
