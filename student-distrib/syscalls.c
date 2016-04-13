// syscalls.c

#include "syscalls.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"

// CONSTANTS
#define PROCESS_KERNEL_STACK_ADDR 0x007ffffc // last location in kernel page that is accessible (according to GDB)
#define EXE_ENTRY_POINT 0x08048000
uint8_t MAGIC_EXE_NUMS[4] = {0x7f, 0x45, 0x4c, 0x46};

// GLOBAL VARIABLES
uint32_t CPID = 0;
pcb_t processes[7];

// File Ops Tables
int32_t no_read (file_t * file, uint8_t * buf, int32_t nbytes) {
    return -1;
}

int32_t no_write(file_t * file, uint8_t * buf, int32_t nbytes) {
    return -1;
}

fileops_t fs_jumptable = {fs_open, fs_read, fs_write, fs_close};
fileops_t stdin_jumptable = {terminal_open, terminal_read, no_write, terminal_close};
fileops_t stdout_jumptable = {terminal_open, no_read, terminal_write, terminal_close};
fileops_t rtc_jumptable = {rtc_open, rtc_read, rtc_write, rtc_close};


// FUNCTION DECLARATIONS
void syscalls_init();
int32_t halt (uint8_t status);
int32_t execute (int8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, void* buf, int32_t nbytes);
int32_t open (const int8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screenstart);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

void syscalls_init() {
    int32_t i;
    for (i = 0; i < MAX_FD; i++) {
        processes[CPID].fd_array[i].flags.in_use = 0;
    }
    processes[CPID].PID = CPID;
    processes[CPID].PPID = 0;
    processes[CPID].running = 1;
}

int32_t halt (uint8_t status) {
    int32_t i;

    for (i = 0; i < MAX_FD; i++) {
        close(i);
    }

    processes[CPID].running = 0;
    CPID = processes[CPID].PPID;
    swap_pages(CPID);
    tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (0x00002000*(CPID-1));

    uint32_t ret = (uint32_t) status;
    haltasm(processes[CPID].ebp, processes[CPID].esp, ret);


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

int32_t execute (int8_t* command) {
    int8_t exename[MAX_FNAME_LEN];
    int32_t i;

    // parse
    if (command == NULL) {
        return -1;
    }
    for (i = 0; command[i] != '\0' && command[i] != ' '; i++) {
        exename[i] = command[i];
    }
    exename[i] = '\0';

    // fetch file
    // uint8_t buf[MAX_FNAME_LEN];
    int32_t fd;
    // int32_t count;
    if ((fd = open(exename)) == -1) {
        return -1;
    }

    // exe check
    uint8_t first_bytes[4];
    if (read(fd, first_bytes, 4) == -1) {
        return -1;
    }

    if (strncmp((int8_t *) MAGIC_EXE_NUMS, (int8_t *) first_bytes, 4)) {
        return -1;
    } 

    if (close(fd) == -1 && CPID != 0) {
        return -1;
    }

    // new pcb
    int32_t old_CPID = CPID;
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

    processes[CPID].fd_array[0].jumptable = &stdin_jumptable;
    processes[CPID].fd_array[1].jumptable = &stdout_jumptable;

    // set up paging
    new_page_directory(CPID);

    // file loader
    if (fs_copy(exename, (uint8_t *) EXE_ENTRY_POINT)) {
        return -1;
    }

    uint8_t new_eip[4];
    uint32_t user_entry = 0;
    new_eip[0] = *((uint8_t *) EXE_ENTRY_POINT + 24);
    new_eip[1] = *((uint8_t *) EXE_ENTRY_POINT + 25);
    new_eip[2] = *((uint8_t *) EXE_ENTRY_POINT + 26);
    new_eip[3] = *((uint8_t *) EXE_ENTRY_POINT + 27);

    for (i = 0; i < 4; i ++) {
        user_entry |= (uint32_t) new_eip[i] << (8*i);
    }

    // save current esp ebp or anything you need in pcb
    int32_t old_esp, old_ebp;
    __asm__("movl %%esp, %0; movl %%ebp, %1"
             :"=g"(old_esp), "=g"(old_ebp) /* outputs (%0 and %1 respectively) */
            );
    processes[old_CPID].esp = old_esp;
    processes[old_CPID].ebp = old_ebp;

    // write tss.esp0/ss0 with new process kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (0x00002000*(CPID-1));
    //tss.esp0 = 0x00800000-(0x00002000*(CPID-1));
    // tss.esp0 = 0x00400000;

    kernel_to_user(user_entry);  

    return 0;
}


int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    if (fd < 0 || fd > MAX_FD || processes[CPID].fd_array[fd].flags.in_use == 0)
        return -1;
    return processes[CPID].fd_array[fd].jumptable->read(&processes[CPID].fd_array[fd], buf, nbytes);
}

int32_t write (int32_t fd, void* buf, int32_t nbytes) {
    if (fd < 0 || fd > MAX_FD || processes[CPID].fd_array[fd].flags.in_use == 0)
        return -1;
    return processes[CPID].fd_array[fd].jumptable->write(&processes[CPID].fd_array[fd], buf, nbytes);
}

int32_t open (const int8_t* filename) {
    dentry_t dentry;
    if (read_dentry_by_name(filename, &dentry))
        return -1;

    int32_t i;
    for (i = 0; i < MAX_FD; i++) {
        if (processes[CPID].fd_array[i].flags.in_use == 0) {
            if (dentry.type == 0) {
                processes[CPID].fd_array[i].jumptable = &rtc_jumptable;
            }
            else
                processes[CPID].fd_array[i].jumptable = &fs_jumptable;
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

int32_t close (int32_t fd) {
    if (fd < 2 || fd > 7 || processes[CPID].fd_array[fd].flags.in_use == 0)
        return -1;
    processes[CPID].fd_array[fd].flags.in_use = 0;
    return processes[CPID].fd_array[fd].jumptable->close(&processes[CPID].fd_array[fd]);
}

int32_t getargs (uint8_t* buf, int32_t nbytes) {
    return -1;
}

int32_t vidmap (uint8_t** screenstart) {
    return -1;
}

int32_t set_handler (int32_t signum, void* handler_address) {
    return -1;
}

int32_t sigreturn (void) {
    return -1;
}
