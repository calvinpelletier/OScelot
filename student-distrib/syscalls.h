#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"
#include "filesys.h"
#include "rtc.h"
#include "terminal.h"

#define MAX_FD        8
#define MAX_PROCESSES 6

/*
 *	Process Control Block used to describe each process. Contains data
 *  pertinent to the current process. WORK IN PROGRESS
 *
 * 	Fields:
 *	fd_array: Array of file descriptors, contains info on each file opened by process.
 * 			  First two files are stdin and stdout, up to 6 more files can be opened after that.
 *	PID: Process number/ID of the current running process.
 * 	PPID: Parent process number/ID of the current running process.
 *  esp: Value of ESP before context switch
 *  ebp: Value of EBP before context switch
 *  running: Boolean to determine if the process is running or not
 *  tss_esp0: Value of ESP0 to store in TSS
 */

typedef struct {
	file_t fd_array[MAX_FD]; // File descriptor array
	uint32_t PID;            
	uint32_t PPID;           
	int32_t esp;
	int32_t ebp;
	uint8_t running; // 0 for no, 1 for yes
	int32_t tss_esp0;
} pcb_t;

extern void syscalls_init();
extern void kernel_to_user(uint32_t user_entry);
extern void haltasm(int32_t ebp, int32_t esp, uint32_t PPID);
extern int32_t exception_halt ();

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
