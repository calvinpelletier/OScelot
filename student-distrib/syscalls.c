// syscalls.c

#include "syscalls.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"

// CONSTANTS
#define PROCESS_KERNEL_STACK_ADDR 0x007ffffc // last location in kernel page that is accessible (according to GDB)
#define EXE_ENTRY_POINT 0x08048000
unsigned char MAGIC_EXE_NUMS[4] = {0x7f, 0x45, 0x4c, 0x46};

// GLOBAL VARIABLES
unsigned int CPID = 0;
pcb_t processes[7];

// File Ops Tables
fileops_t fs_jumptable = {fs_open, fs_read, fs_write, fs_close};
fileops_t term_jumptable = {terminal_open, terminal_read, terminal_write, terminal_close};
//fileops_t rtc_jumptable = {rtc_open, rtc_read, rtc_write, rtc_close};


// FUNCTION DECLARATIONS
void syscalls_init(void);
int halt (unsigned char status);
int execute (const char* command);
int read (int fd, void* buf, int nbytes);
int write (int fd, void* buf, int nbytes);
int open (const char* filename);
int close (int fd);
int getargs (unsigned char* buf, int nbytes);
int vidmap (unsigned char** screenstart);
int set_handler (int signum, void* handler_address);
int sigreturn (void);

void syscalls_init(void) {
    int i;

    for (i = 0; i < MAX_FD; i++) {
        processes[CPID].fd_array[i].flags.in_use = 0;
    }
    processes[CPID].PID = CPID;
    processes[CPID].PPID = 0;
    processes[CPID].running = 1;
}

int halt (unsigned char status) {
    int i;

    for (i = 0; i < MAX_FD; i++) {
        close(i);
    }

    processes[CPID].running = 0;
    CPID = processes[CPID].PPID;

    haltasm(processes[CPID].ebp, processes[CPID].esp);

    // __asm__("movl %0, %%ebp"
    //         :
    //         : "r" (processes[CPID].ebp)
    //         : "memory");

    // __asm__("movl %0, %%esp"
    //         :
    //         : "r" (processes[CPID].esp)
    //         : "memory");

    // __asm__("jmp end_execute");

    return 0;
}


int execute (const char* command) {
    unsigned char exename[MAX_FNAME_LEN];
    int i;

    // parse
    if (command == NULL) {
        return -1;
    }
    for (i = 0; command[i] != '\0' && command[i] != ' '; i++) {
        exename[i] = command[i];
    }
    exename[i] = '\0';

    printf("check0\n");

    // fetch file
    int fd;
    if ((fd = open((char *) exename)) == -1) {
        return -1;
    }

    printf("check1\n");

    // exe check
    unsigned char first_bytes[4];
    if (read(fd, first_bytes, 4) == -1) {
        return -1;
    }

    printf("check1.1\n");

    if (strncmp((char *) MAGIC_EXE_NUMS, (char *) first_bytes, 4)) {
        return -1;
    }

    printf("check2\n");

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

    processes[CPID].fd_array[0].jumptable = &term_jumptable;
    processes[CPID].fd_array[1].jumptable = &term_jumptable;


    printf("check3\n");

    // set up paging
    new_page_directory(CPID);

    printf("check4\n");

    // file loader
    if (fs_copy((char *) exename, (unsigned char *) EXE_ENTRY_POINT)) {
        return -1;
    }

    unsigned char new_eip[4];
    unsigned int user_entry = 0;
    new_eip[0] = *((unsigned char *) EXE_ENTRY_POINT + 24);
    new_eip[1] = *((unsigned char *) EXE_ENTRY_POINT + 25);
    new_eip[2] = *((unsigned char *) EXE_ENTRY_POINT + 26);
    new_eip[3] = *((unsigned char *) EXE_ENTRY_POINT + 27);

    for (i = 0; i < 4; i ++) {
        user_entry |= (unsigned int) new_eip[i] << (8*i);
    }

    printf("check5\n");

    // save current esp ebp or anything you need in pcb
    int old_esp, old_ebp;
    __asm__("movl %%esp, %0; movl %%ebp, %1"
             :"=g"(old_esp), "=g"(old_ebp) /* outputs (%0 and %1 respectively) */
            );
    processes[old_CPID].esp = old_esp;
    processes[old_CPID].ebp = old_ebp;

    printf("check6\n");

    // write tss.esp0/ss0 with new process kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (0x00002000*(CPID-1));
    //tss.esp0 = 0x00800000-(0x00002000*(CPID-1));
    // tss.esp0 = 0x00400000;

    kernel_to_user(user_entry);    

    printf("check7\n");

    // asm volatile("cli; 
    //               movl %0, %%eax; 
    //               movw %%ax, %%ds; 
    //               movw %%ax, %%es; 
    //               movw %%ax, %%fs; 
    //               movw %%ax, %%gs; 
    //               movl %%esp, %%ebx; 
    //               pushl %%eax; 
    //               pushl %%ebx; 
    //               pushf; 
    //               popl %%eax; 
    //               orl $0x200, %%eax; 
    //               pushl %%eax; 
    //               pushl %1; 
    //               pushl %2"
    //               :
    //               : "r"(USER_DS), "r"(USER_CS), "r"(0x080482e8)
    //             );
    // asm volatile("iret; end_execute:");

    // push artificial iret context onto stack
    /*__asm__("pushf"); // push FLAGS

    __asm__("push %0"
           : // nothing here
           : "r"(USER_CS)
           ); // push CS

    __asm__("push %0"
           : // nothing here
           : "r"(EXE_ENTRY_POINT)
           ); // push EIP

    // printf("check8\n");

    // iret
    __asm__("iret; end_execute:"
            ); //  most likely incorrect
*/
    return 0;
}


int read (int fd, void* buf, int nbytes) {
    if (fd < 0 || fd > MAX_FD)
        return -1;
    return processes[CPID].fd_array[fd].jumptable->read(&processes[CPID].fd_array[fd], buf, nbytes);
}

int write (int fd, void* buf, int nbytes) {
    if (fd < 0 || fd > MAX_FD)
        return -1;
    return processes[CPID].fd_array[fd].jumptable->write(&processes[CPID].fd_array[fd], buf, nbytes);
}

int open (const char* filename) {
    dentry_t dentry;
    if (read_dentry_by_name(filename, &dentry))
        return -1;

    int i;
    for (i = 0; i < MAX_FD; i++) {
        if (processes[CPID].fd_array[i].flags.in_use == 0) {
            // if (dentry.type == 0) {
            //     processes[CPID].fd_array[i].jumptable = rtc_jumptable;
            // }
            processes[CPID].fd_array[i].jumptable = & fs_jumptable;
            if (processes[CPID].fd_array[i].jumptable->open())
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
    return processes[CPID].fd_array[fd].jumptable->close(&processes[CPID].fd_array[fd]);
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
