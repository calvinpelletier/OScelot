// syscalls.c

#include "syscalls.h"
#include "lib.h"
#include "filesys.h"

// CONSTANTS
#define MAX_FNAME_LEN 32;
unsigned char MAGIC_EXE_NUMS[4] = {0x7f, 0x45, 0x4c, 0x46};


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

    // fetch file
    unsigned char buf[MAX_FNAME_LEN];
    int fd, count;
    if ((fd = fs_open(exename) == -1) {
        return -1;
    }

    // exe check
    unsigned char first_bytes[4];
    if (fs_read(fd, first_bytes, 4)) {
        return -1;
    }
    if (strncmp(MAGIC_EXE_NUMS, first_bytes, 4)) {
        return -1;
    }


    // set up paging
    // file loader
    // new pcb

    // save current esp ebp or anything you need in pcb
    int old_esp, old_ebp;
    __asm__("mv %%esp, %0;
             mv %%ebp, %1;"
             :"=r"(old_esp), "=r"(old_ebp) // outputs (%0 and %1 respectively)
             :
             :
            );
    // TODO store old esp and ebp

    // write tss.esp0/ss0 with new process kernel stack
    tss.ss0 = USER_DS;
    // TODO: tss.esp0 = ;

    // push artificial iret context onto stack
    __asm__("pushf"); // push FLAGS
    __asm__("push "); // push CS
    __asm__("push "); // push EIP
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
