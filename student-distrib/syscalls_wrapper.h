// syscalls_wrapper.h

#ifndef SYSCALLS_WRAPPER_H
#define SYSCALLS_WRAPPER_H

extern int halt (unsigned char status);
extern int execute (unsigned char* command);
extern int read (int fd, void* buf, int nbytes);
extern int write (int fd, const void* buf, int nbytes);
extern int open (const unsigned char* filename);
extern int close (int fd);


#endif
