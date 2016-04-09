#ifndef SYSCALL_H
#define SYSCALL_H

extern int halt (unsigned char status);
extern int execute (unsigned char* command);
extern int read (int fd, void* buf, int nbytes);
extern int write (int fd, const void* buf, int nbytes);
extern int open (const unsigned char* filename);
extern int close (int fd);
extern int getargs (unsigned char* buf, int nbytes);
extern int vidmap (unsigned char** screenstart);
extern int set_handler (int signum, void* handler_address);
extern int sigreturn (void);

#endif
