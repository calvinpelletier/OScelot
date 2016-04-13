#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"
#include "filesys.h"
#include "rtc.h"
#include "terminal.h"

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
	uint32_t PID;
	uint32_t PPID;
	int32_t esp;
	int32_t ebp;
	uint8_t running; // 0 for no, 1 for yes
} pcb_t;

extern void syscalls_init();
extern void kernel_to_user(uint32_t user_entry);
extern void haltasm(int32_t ebp, int32_t esp);

// System Calls
extern int32_t halt (uint8_t status);
extern int32_t execute (int8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write (int32_t fd, void* buf, int32_t nbytes);
extern int32_t open (const int8_t* filename);
extern int32_t close (int32_t fd);
extern int32_t getargs (uint8_t* buf, int32_t nbytes);
extern int32_t vidmap (uint8_t** screenstart);
extern int32_t set_handler (int32_t signum, void* handler_address);
extern int32_t sigreturn (void);



#endif
