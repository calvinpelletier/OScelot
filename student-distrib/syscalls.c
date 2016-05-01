// syscalls.c

#include "syscalls.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"
#include "syscalls_asm.h"

// CONSTANTS
#define PROCESS_KERNEL_STACK_ADDR 0x007ffffc // Last location in kernel page that is accessable
#define EXE_ENTRY_POINT           0x08048000 // Entry point for executables in virtual memory
#define STACK_SIZE                0x00002000 // Size of kernel stack
#define VIRT_ADDR_BYTE_1          24
#define VIRT_ADDR_BYTE_2          25         /* Bytes 24-27 of the EXE hold virtual address of first */
#define VIRT_ADDR_BYTE_3          26         /* instruction to be executed.                          */
#define VIRT_ADDR_BYTE_4          27

uint8_t MAGIC_EXE_NUMS[4] = {0x7f, 0x45, 0x4c, 0x46};

// GLOBAL VARIABLES
uint32_t CPID = 0;
pcb_t processes[MAX_PROCESSES + 1];
uint32_t active_processes[NUM_TERMINALS]; // active process for each terminal
uint8_t needs_to_be_halted[NUM_TERMINALS]; // flag for letting the task_switch know that we need to halt an active processes

// File Ops Tables
int32_t no_read (file_t * file, uint8_t * buf, int32_t nbytes) {
    return -1;
}

int32_t no_write(file_t * file, uint8_t * buf, int32_t nbytes) {
    return -1;
}

static fileops_t fs_jumptable = {fs_open, fs_read, fs_write, fs_close};
static fileops_t stdin_jumptable = {terminal_open, terminal_read, no_write, terminal_close};
static fileops_t stdout_jumptable = {terminal_open, no_read, terminal_write, terminal_close};
static fileops_t rtc_jumptable = {rtc_open, rtc_read, rtc_write, rtc_close};


// FUNCTION DECLARATIONS
void syscalls_init();
void task_switch();
int execute_base_shell(unsigned char terminal);
int32_t halt (uint8_t status);
int32_t execute (int8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, void* buf, int32_t nbytes);
int32_t open (const int8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (int8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screenstart);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

/*
 * syscalls_init
 *   DESCRIPTION:  Initializes PCB structs
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites PCB structs
 */
void syscalls_init() {
    int32_t i;

    /* Initialize the PCB with the pertinent information */
    for (i = 0; i < MAX_FD; i++) {
        if (i == 0 || i == 1) {
            processes[CPID].fd_array[i].flags.in_use = 1;
        } else {
            processes[CPID].fd_array[i].flags.in_use = 0;
        }
    }
    processes[CPID].fd_array[0].jumptable = &stdin_jumptable;
    processes[CPID].fd_array[1].jumptable = &stdout_jumptable;
    processes[CPID].PID = CPID;
    processes[CPID].PPID = 0;
    processes[CPID].running = 1;
    processes[CPID].args[0] = '\0';
    processes[CPID].args_size = 0;
    processes[CPID].terminal = 0; // so that the first shell is in terminal 0

    // multitasking stuff
    for (i = 0; i < NUM_TERMINALS; i++) {
        active_processes[i] = 0;
        needs_to_be_halted[i] = 0;
    }
}

/*
 * task_switch
 *   DESCRIPTION:  switches the running task to a different active process
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: context switch
 */
void task_switch() {
    cli();

    // check if we need to halt this process
    if (needs_to_be_halted[processes[CPID].terminal]) {
        needs_to_be_halted[processes[CPID].terminal] = 0;
        clear();
        set_pos(0, 0);
        puts("391OS> ");
        exception_halt();
        return;
    }

    // find next active process
    int old_CPID = CPID;
    int i = processes[old_CPID].terminal;
    do {
        i++;
    } while (active_processes[i % NUM_TERMINALS] == 0);
    CPID = active_processes[i % NUM_TERMINALS];

    // return if there are no other active processes
    if (CPID == old_CPID) {
        return;
    }

    // adjust video memory
    if (processes[CPID].terminal == cur_terminal) {
        set_video_context(ACTIVE_CONTEXT);
    } else {
        set_video_context(processes[CPID].terminal);
    }

    // adjust user mapping into video memory
    // for old process
    if (processes[old_CPID].using_video_mem) {
        if (processes[old_CPID].terminal == cur_terminal) {
            new_page_directory_entry(old_CPID, USER_PAGE_BOTTOM, VIDEO, 0, 3);
        } else {
            if (processes[old_CPID].terminal == 0) {
                new_page_directory_entry(old_CPID, USER_PAGE_BOTTOM, VIDEO_0, 0, 3);
            } else if (processes[old_CPID].terminal == 1) {
                new_page_directory_entry(old_CPID, USER_PAGE_BOTTOM, VIDEO_1, 0, 3);
            } else if (processes[old_CPID].terminal == 2) {
                new_page_directory_entry(old_CPID, USER_PAGE_BOTTOM, VIDEO_2, 0, 3);
            }
        }
    }
    // and new process
    if (processes[CPID].using_video_mem) {
        if (processes[CPID].terminal == cur_terminal) {
            new_page_directory_entry(CPID, USER_PAGE_BOTTOM, VIDEO, 0, 3);
        } else {
            if (processes[CPID].terminal == 0) {
                new_page_directory_entry(CPID, USER_PAGE_BOTTOM, VIDEO_0, 0, 3);
            } else if (processes[CPID].terminal == 1) {
                new_page_directory_entry(CPID, USER_PAGE_BOTTOM, VIDEO_1, 0, 3);
            } else if (processes[CPID].terminal == 2) {
                new_page_directory_entry(CPID, USER_PAGE_BOTTOM, VIDEO_2, 0, 3);
            }
        }
    }

    // save esp/ebp
    int old_esp, old_ebp;
    __asm__("movl %%esp, %0; movl %%ebp, %1"
             :"=g"(old_esp), "=g"(old_ebp) /* outputs */
            );
    processes[old_CPID].esp_switch = old_esp;
    processes[old_CPID].ebp_switch = old_ebp;

    /* Write to TSS SS0 and ESP0 fields with new kernel stack info */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (STACK_SIZE*(CPID-1));

    // switch page directories
    swap_pages(CPID);

    // load new esp/ebp and continue executing from new context
    asm volatile("movl %0, %%ebp;\
                  movl %1, %%esp;\
                  sti"
                  :
                  : "g"(processes[CPID].ebp_switch), "g"(processes[CPID].esp_switch)
              );
    return; // should switch to new context
}

/*
 * execute_base_shell
 *   DESCRIPTION:  starts the base shell in a given terminal
 *   INPUTS:       terminal number 0-2
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: same as execute
 */
int execute_base_shell(unsigned char terminal) {
    cli();

    int32_t i;
    uint8_t new_eip[4];
    uint32_t user_entry;

    // initialize terminal struct
    terminal_init(terminal);

    /* Create a new PCB for the process and update relevant fields */
    CPID = 0;

    /* If the current process is running, update CPID to set up next PCB */
    while (processes[CPID].running) {
        CPID++;
        if (CPID > MAX_PROCESSES) {
            CPID = 0; // reset CPID
            return -2;       // return value to indicate program found, but could not execute
        }
    }

    /* Set file descriptors */
    for (i = 0; i < MAX_FD; i++) {

        /* FD 0 and FD 1 are stdin and stdout so they should be set to in-use on init */
        if (i == 0 || i == 1) {
            processes[CPID].fd_array[i].flags.in_use = 1;
        } else {
            processes[CPID].fd_array[i].flags.in_use = 0;
        }
    }

    /* Update current process PCB struct fields */
    processes[CPID].PID = CPID;
    processes[CPID].PPID = 0; // kernel
    processes[CPID].running = 1;

    processes[CPID].fd_array[0].jumptable = &stdin_jumptable;
    processes[CPID].fd_array[1].jumptable = &stdout_jumptable;

    processes[CPID].args[0] = '\0';  // play this safe, null terminate everywhere (in halt, in getargs as well)
    processes[CPID].args_size = 0;

    // multitasking stuff
    processes[CPID].active = 1;
    processes[0].active = 0;
    processes[CPID].terminal = terminal;
    active_processes[processes[CPID].terminal] = CPID;
    processes[CPID].using_video_mem = 0;

    /* Set up paging for current process */
    new_page_directory(CPID);

    /* Load the file into memory */
    if (fs_copy("shell", (uint8_t *) EXE_ENTRY_POINT)) {
        return -1;
    }

    /* Determine the entry point for the executable */
    new_eip[0] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_1);
    new_eip[1] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_2);
    new_eip[2] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_3);
    new_eip[3] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_4);

    user_entry = 0;
    for (i = 0; i < 4; i ++) {
        user_entry |= (uint32_t) new_eip[i] << (8*i);
    }

    /* Write to TSS SS0 and ESP0 fields with new kernel stack info */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (STACK_SIZE*(CPID-1));

    /* Context switch */
    kernel_to_user(user_entry);

    return 0;
}


/*
 * halt
 *   DESCRIPTION:  Terminates a process. This function should never return
 *                 to the caller.
 *   INPUTS:       status - status value to be returned to parent process
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites PCB structs
 */
int32_t halt (uint8_t status) {
    int32_t i;

    /* Close all file descriptors */
    for (i = 0; i < MAX_FD; i++) {
        close(i);
    }

    /* update process info */
    processes[CPID].running = 0;
    processes[CPID].active = 0;
    unsigned char terminal = processes[CPID].terminal;
    CPID = processes[CPID].PPID;
    active_processes[processes[CPID].terminal] = CPID;
    processes[CPID].active = 1;
    processes[CPID].args[0] = '\0';
    processes[CPID].args_size = 0;

    /* If we attempt to halt the last process, we re-launch shell instead */
    if (CPID == 0) {
        swap_pages(CPID);
        printf("Cannot close last process! Restarting shell...\n");
        execute_base_shell(terminal);
        return 0;
    } else {
        swap_pages(CPID);
        tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (STACK_SIZE*(CPID-1));
    }

    uint32_t ret = (uint32_t) status;
    haltasm(processes[CPID].ebp_execute, processes[CPID].esp_execute, ret);

    return 0;
}

/*
 * exception_halt
 *   DESCRIPTION:  Terminates a process that raised an exception. This function should never return
 *                 to the caller.
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites PCB structs
 */
int32_t exception_halt () {
    int32_t i;

    /* Close all file descriptors */
    for (i = 0; i < MAX_FD; i++) {
        close(i);
    }

    /* Set the current process running flag to 0 and update CPID field */
    processes[CPID].running = 0;
    processes[CPID].active = 0;
    unsigned char terminal = processes[CPID].terminal;
    CPID = processes[CPID].PPID;
    active_processes[processes[CPID].terminal] = CPID;
    processes[CPID].active = 1;
    processes[CPID].args[0] = '\0';
    processes[CPID].args_size = 0;

    /* If we attempt to halt the last process, we re-launch shell instead */
    if (CPID == 0) {
        swap_pages(CPID);
        printf("Cannot close last process! Restarting shell...\n");
        execute_base_shell(terminal);
        return 0;
    } else {
        swap_pages(CPID);
        tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (STACK_SIZE*(CPID-1));
    }
    haltasm(processes[CPID].ebp_execute, processes[CPID].esp_execute, 256);

    return 0;
}

/*
 * execute
 *   DESCRIPTION:  Loads and executes a new program, handing off the processor
 *                 to the new program until it terminates.
 *   INPUTS:       command - space-separated sequence of words; first word is
 *                           the file name, the rest is provided to the new
 *                           program via getargs() system call
 *   OUTPUTS:      none
 *   RETURN VALUE: -1 if command cannot be executed, 256 if program dies by exception
 *                 or a value 0-255 if program executes a halt() system call
 *   SIDE EFFECTS: Overwrites PCB structs and memory
 */
int32_t execute (int8_t* command) {
    cli();

    int8_t exename[MAX_FNAME_LEN];
    int32_t i, j;
    int32_t old_CPID;
    int32_t fd;
    uint8_t first_bytes[4];
    uint8_t new_eip[4];
    int8_t args[BUFFER_SIZE];
    uint32_t args_size;
    uint32_t user_entry;
    int32_t old_esp, old_ebp;

    /* Parse command passed into execute() */
    if (command == NULL) {
        return -1;
    }

    i = 0;
    while (command[i] != '\0' && command[i] != ' ') {
      if (i >= MAX_FNAME_LEN)
        return -1;
      exename[i] = command[i];
      i++;
    }

    exename[i] = '\0';

    while (command[i] == ' ') {
        i++;
    }

    args_size = 0;
    for (j = i; command[j] != '\0' && j < (BUFFER_SIZE - i);  j++) {
        if (CPID < MAX_PROCESSES) {
            args[j - i] = command[j];
            args_size++;
        }
    }

    /* Fetch the file executable */
    if ((fd = open(exename)) == -1) {
        return -1;
    }

    /* Check to make sure the file is executable */
    if (read(fd, first_bytes, 4) == -1) {
        return -1;
    }

    if (close(fd) == -1 && CPID != 0) {
        return -1;
    }

    if (strncmp((int8_t *) MAGIC_EXE_NUMS, (int8_t *) first_bytes, 4)) {
        return -1;
    }

    /* Create a new PCB for the process and update relevant fields */
    old_CPID = CPID;
    CPID = 0;

    /* If the current process is running, update CPID to set up next PCB */
    while (processes[CPID].running) {
        CPID++;
        if (CPID > MAX_PROCESSES) {
            CPID = old_CPID; // reset CPID
            return -2;       // return value to indicate program found, but could not execute
        }
    }

    /* Set file descriptors */
    for (i = 0; i < MAX_FD; i++) {

        /* FD 0 and FD 1 are stdin and stdout so they should be set to in-use on init */
        if (i == 0 || i == 1) {
            processes[CPID].fd_array[i].flags.in_use = 1;
        } else {
            processes[CPID].fd_array[i].flags.in_use = 0;
        }
    }

    /* Update current process PCB struct fields */
    processes[CPID].PID = CPID;
    processes[CPID].PPID = old_CPID;
    processes[CPID].running = 1;

    processes[CPID].fd_array[0].jumptable = &stdin_jumptable;
    processes[CPID].fd_array[1].jumptable = &stdout_jumptable;

    memcpy(processes[CPID].args, args, args_size);
    processes[CPID].args[args_size] = '\0';  // play this safe, null terminate everywhere (in halt, in getargs as well)
    processes[CPID].args_size = args_size;

    // multitasking stuff
    processes[CPID].active = 1;
    processes[old_CPID].active = 0;
    processes[CPID].terminal = processes[old_CPID].terminal; // inherit from parent
    active_processes[processes[CPID].terminal] = CPID;
    processes[CPID].using_video_mem = 0;

    /* Set up paging for current process */
    new_page_directory(CPID);

    /* Load the file into memory */
    if (fs_copy(exename, (uint8_t *) EXE_ENTRY_POINT)) {
        return -1;
    }

    /* Determine the entry point for the executable */
    new_eip[0] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_1);
    new_eip[1] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_2);
    new_eip[2] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_3);
    new_eip[3] = *((uint8_t *) EXE_ENTRY_POINT + VIRT_ADDR_BYTE_4);

    user_entry = 0;
    for (i = 0; i < 4; i ++) {
        user_entry |= (uint32_t) new_eip[i] << (8*i);
    }

    /* Save current ESP and EBP into PCB */
    __asm__("movl %%esp, %0; movl %%ebp, %1"
             :"=g"(old_esp), "=g"(old_ebp) /* outputs */
            );
    processes[old_CPID].esp_execute = old_esp;
    processes[old_CPID].ebp_execute = old_ebp;

    /* Write to TSS SS0 and ESP0 fields with new kernel stack info */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PROCESS_KERNEL_STACK_ADDR - (STACK_SIZE*(CPID-1));

    /* Context switch */
    kernel_to_user(user_entry);

    return 0;
}

/*
 * read
 *   DESCRIPTION:  Reads data from the keyboard, a file, device (RTC) or
 *                 directory.
 *   INPUTS:       fd - file descriptor
 *                 buf - buffer to store read info
 *                 nbytes - number of bytes read
 *   OUTPUTS:      none
 *   RETURN VALUE: 0 if successful, -1 if not
 *   SIDE EFFECTS: Can overwrite different buffers depending on which jump table is used
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    if (fd < 0 || fd >= MAX_FD || processes[CPID].fd_array[fd].flags.in_use == 0)
        return -1;

    return processes[CPID].fd_array[fd].jumptable->read(&processes[CPID].fd_array[fd], buf, nbytes);
}

/*
 * write
 *   DESCRIPTION:  Writes data to the terminal or to a device, NOT files.
 *   INPUTS:       fd - file descriptor
 *                 buf - buffer to read from
 *                 nbytes - number of bytes written
 *   OUTPUTS:      none
 *   RETURN VALUE: 0 if successful, -1 if not
 *   SIDE EFFECTS: Can overwrite different buffers depending on which jump table is used
 */
int32_t write (int32_t fd, void* buf, int32_t nbytes) {
    if (fd < 0 || fd >= MAX_FD || processes[CPID].fd_array[fd].flags.in_use == 0)
        return -1;

    return processes[CPID].fd_array[fd].jumptable->write(&processes[CPID].fd_array[fd], buf, nbytes);
}

/*
 * open
 *   DESCRIPTION:  Provides access to the file system. Finds the appropriate directory entry
 *                 and allocates an unused file descriptor along with the pertinent data.
 *   INPUTS:       filename - directory entry to find
 *   OUTPUTS:      none
 *   RETURN VALUE: 0 if successful, -1 if not
 *   SIDE EFFECTS: Overwrites PCB structs
 */
int32_t open (const int8_t* filename) {
    dentry_t dentry;
    int32_t i;

    if (!filename)
        return -1; // NULL

    if (read_dentry_by_name(filename, &dentry))
        return -1;

    /* Check for non-used file descriptors and populate one FD with the file info */
    for (i = 0; i < MAX_FD; i++) {
        if (processes[CPID].fd_array[i].flags.in_use == 0) {
            if (dentry.type == 0) {
                processes[CPID].fd_array[i].jumptable = &rtc_jumptable;
            } else {
                processes[CPID].fd_array[i].jumptable = &fs_jumptable;
            }

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

/*
 * close
 *   DESCRIPTION:  Closes the specified file descriptor and makes it available for
 *                 more open() calls.
 *   INPUTS:       filename - directory entry to find
 *   OUTPUTS:      none
 *   RETURN VALUE: 0 if successful, -1 if not
 *   SIDE EFFECTS: Overwrites PCB structs
 */
int32_t close (int32_t fd) {

    /* The user should not be able to close FD 0 or 1 */
    if (fd < 2 || fd >= MAX_FD || processes[CPID].fd_array[fd].flags.in_use == 0)
        return -1;

    processes[CPID].fd_array[fd].flags.in_use = 0;

    return processes[CPID].fd_array[fd].jumptable->close(&processes[CPID].fd_array[fd]);
}

/*
 * getargs
 *   DESCRIPTION:  returns the arguments that the process was launched with
 *   INPUTS:       buffer, num bytes of buffer
 *   OUTPUTS:      arguments as a string
 *   RETURN VALUE: 0 if successful, -1 if not
 *   SIDE EFFECTS: Overwrites PCB structs
 */
int32_t getargs (int8_t* buf, int32_t nbytes) {
    if (buf == NULL || processes[CPID].args_size >= (nbytes-1)) {
        return -1;
    }

    memset(buf, 0, nbytes);
    memcpy(buf, processes[CPID].args, processes[CPID].args_size);
    buf[processes[CPID].args_size] = '\0';

    return 0;
}

/*
 * vidmap
 *   DESCRIPTION:  maps video memory into user space
 *   INPUTS:       location in user memory
 *   OUTPUTS:      none
 *   RETURN VALUE: 0 if successful, -1 if not
 *   SIDE EFFECTS: changes page directory
 */
int32_t vidmap (uint8_t** screenstart) {
    if (screenstart == NULL) {
        return -1;
    }

    if ((int32_t) screenstart > (USER_PAGE_BOTTOM-4) || (int32_t) screenstart < PROGRAM_IMAGE) {
        return -1;
    }

    processes[CPID].using_video_mem = 1;

    uint32_t user_video_addr = USER_PAGE_BOTTOM;

    // map to correct video memory
    if (processes[CPID].terminal == cur_terminal) {
        if (new_page_directory_entry(CPID, user_video_addr, VIDEO, 0, 3)) {
            return -1;
        }
    } else {
        if (processes[CPID].terminal == 0) {
            if (new_page_directory_entry(CPID, user_video_addr, VIDEO_0, 0, 3)) {
                return -1;
            }
        } else if (processes[CPID].terminal == 1) {
            if (new_page_directory_entry(CPID, user_video_addr, VIDEO_1, 0, 3)) {
                return -1;
            }
        } else if (processes[CPID].terminal == 2) {
            if (new_page_directory_entry(CPID, user_video_addr, VIDEO_2, 0, 3)) {
                return -1;
            }
        }
    }

    *screenstart = (uint8_t *) user_video_addr;

    return 0;
}

/*
 * set_handler
 *   DESCRIPTION:  does nothing
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t set_handler (int32_t signum, void* handler_address) {
    return -1;
}

/*
 * sigreturn
 *   DESCRIPTION:  does nothing
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t sigreturn (void) {
    return -1;
}
