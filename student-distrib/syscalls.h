#ifndef SYSCALL_H
#define SYSCALL_H

extern int halt_helper (unsigned char status);
extern int execute_helper (unsigned char* command);
extern int read_helper (int fd, void* buf, int nbytes);
extern int write_helper (int fd, const void* buf, int nbytes);
extern int open_helper (const unsigned char* filename);
extern int close_helper (int fd);
extern int getargs_helper (unsigned char* buf, int nbytes);
extern int vidmap_helper (unsigned char** screenstart);
extern int set_handler_helper (int signum, void* handler_address);
extern int sigreturn_helper (void);

#endif
