// syscalls.c

#include "syscalls.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"

// CONSTANTS
#define PROCESS_KERNEL_STACK_ADDR 0x08000000
#define EXE_ENTRY_POINT 0x08048000
unsigned char MAGIC_EXE_NUMS[4] = {0x7f, 0x45, 0x4c, 0x46};

// GLOBAL VARIABLES
unsigned int CPID = 0;
pcb_t processes[7];

// File Ops Tables
fileops_t fs_jumptable = {fs_open, fs_read, fs_write, fs_close};
fileops_t rtc_jumptable = {rtc_open, rtc_read, rtc_write, rtc_close};

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
    int i;

    for (i = 0; i < MAX_FD; i++) {
        close(i);
    }

    __asm__("movl %0, %%ebp"
            :
            : "r" (processes[CPID].ebp))
            : "memory");

    __asm__("movl %0, %%esp"
            :
            : "r" (processes[CPID].esp))
            : "memory");

    __asm__("jmp end_execute");

    return 0;
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

    // new pcb
    int old_CPID = CPID;
    CPID = 0;
    while (processes[CPID].running) {
        CPID++;
        if (CPID > 6) {
            return -1;
        }
    }
    for (i = 0; i < MAX_FD; i++) {
        if (i == 0 || i == 1) {
            processes[CPID].fd_array[i].flags.in_use = 1;
        } else {
            processes[CPID].fd_array[i].flags.in_use = 0;
        }
    }
    processes[CPID].PID = CPID;
    processes[CPID].PPID = old_CPID;
    processes[CPID].running = 1;

    // set up paging
    unsigned int phys_addr = FOUR_MB * (CPID + 1);
    new_page(phys_addr, CPID);

    // file loader

    // save current esp ebp or anything you need in pcb
    int old_esp, old_ebp;
    __asm__("mv %%esp, %0;
             mv %%ebp, %1;"
             :"=r"(old_esp), "=r"(old_ebp) // outputs (%0 and %1 respectively)
             :
             :
            );
    processes[old_CPID].esp = old_esp;
    processes[old_CPID].ebp = old_ebp;

    // write tss.esp0/ss0 with new process kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PROCESS_KERNEL_STACK_ADDR;

    // push artificial iret context onto stack
    __asm__("pushf"); // push FLAGS

    __asm__("push %0;"
           : // nothing here
           : "r"(USER_CS)
           : // nothing here
           ); // push CS

    __asm__("push %0;"
           : // nothing here
           : "r"(EXE_ENTRY_POINT)
           : // nothing here
           ); // push EIP

    // iret
    __asm__("iret;
            end_execute:"
            :
            ); //  most likely incorrect

    return 0;
}

int read (int fd, void* buf, int nbytes) {
    if (fd < 0 || fd > MAX_FD)
        return -1;
    return processes[CPID].fd_array[fd].jumptable.read((file_t *) processes[CPID].fd_array[i], buf, nbytes);
}

int write (int fd, const void* buf, int nbytes) {
    if (fd < 0 || fd > MAX_FD)
        return -1;
    return processes[CPID].fd_array[fd].jumptable.write((file_t *) processes[CPID].fd_array[i], buf, nbytes);
}

int open (const unsigned char* filename) {
    dentry_t dentry;
    if (read_dentry_by_name(filename, &dentry))
        return -1;

    int i;
    for (i = 0; i < MAX_FD; i++) {
        if (processes[CPID].fd_array[i].flags.in_use == 0) {
            if (dentry.type == 0) {
                processes[CPID].fd_array[i].jumptable = rtc_jumptable;
            } 
            else {
                processes[CPID].fd_array[i].jumptable = fs_jumptable;
            } 
            if (processes[CPID].fd_array[i].jumptable.open())
                return -1;
            processes[CPID].fd_array[i].inode = dentry.inode;
            processes[CPID].fd_array[i].position = 0;
            processes[CPID].fd_array[i].filetype = dentry.type;
            processes[CPID].fd_array[i].flags.read_only = 1;
            processes[CPID].fd_array[i].flags.write_only = 0;
            processes[CPID].fd_array[i].flags.in_use = 1;
            return i;
        }
    }

    return -1;
}

int close (int fd) {
    if (fd < 2 || fd > 7)
        return -1;
    processes[CPID].fd_array[fd].flags.in_use = 0;
    return processes[CPID].fd_array[fd].jumptable.close((file_t *) processes[CPID].fd_array[i]);
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


void fd_init ()
{
    // initialize file descriptor array
    int i;
    for (i = 0; i < MAX_FD; i++) {
        if (i < 2)
            processes[CPID].fd_array[i].flags.in_use = 1;
        else
            processes[CPID].fd_array[i].flags.in_use = 0;
    }
}
