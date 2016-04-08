// syscalls.c

#include "syscalls.h"
#include "lib.h"

// FUNCTION DECLARATIONS
int halt_helper (unsigned char status);
int execute_helper (unsigned char* command);
int read_helper (int fd, void* buf, int nbytes);
int write_helper (int fd, const void* buf, int nbytes);
int open_helper (const unsigned char* filename);
int close_helper (int fd);
int getargs_helper (unsigned char* buf, int nbytes);
int vidmap_helper (unsigned char** screenstart);
int set_handler_helper (int signum, void* handler_address);
int sigreturn_helper (void);
