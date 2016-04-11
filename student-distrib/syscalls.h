#ifndef SYSCALL_H
#define SYSCALL_H

#include "filesys.h"
#define MAX_FD 8

/*
 *	Process Control Block used to describe each process. Contains data
 *  pertinent to the current process. WORK IN PROGRESS
 *
 * 	Fields:
 *	fd_array: Array of file descriptors, contains info on each file opened by process.
 * 			  First two files are stdin and stdout, up to 6 more files can be opened after that.
 *	PID: Process number/ID of the current running process.
 * 	PPID: Parent process number/ID of the current running process.
 *
 */

typedef struct {
	file_t fd_array[MAX_FD];
	unsigned char PID;
	unsigned char PPID;
	int esp;
	int ebp;
	unsigned char running; // 0 for no, 1 for yes
} pcb_t;

// System Calls
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
