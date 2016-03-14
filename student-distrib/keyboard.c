// keyboard.c

#include "keyboard.h"
#include "lib.h"


// CONSTANTS
// keyboard ports
#define KEYBOARD_DATA 0x60
#define KEYBOARD_CMD 0x64 // command when writing
#define KEYBOARD_STATUS 0x64  // status when reading

// keyboard commands
#define DISABLE_PORT1 0xAD
#define DISABLE_PORT2 0xA7
#define READ_FROM_CONFIG 0x20
#define WRITE_TO_CONFIG 0x60
#define TEST_DEVICE 0xAA
#define TEST_PORT1 0xAB
#define ENABLE_PORT1 0xAE


// FUNCTION DECLARATIONS
int keyboard_init(void);


// GLOBAL FUNCTIONS
/*
keyboard_init
    DESCRIPTION: initializes keyboard controller
    INPUT: none
    OUTPUT: none
    RETURNS: 0 for success, -1 for failure
*/
int keyboard_init(void) {
    outb(DISABLE_PORT1, KEYBOARD_CMD);
    outb(DISABLE_PORT2, KEYBOARD_CMD); // will be ignored if there is no port 2
    inb(KEYBOARD_DATA); // flushes output buffer

    // set config
    outb(READ_FROM_CONFIG, KEYBOARD_DATA);
    unsigned char config = inb(KEYBOARD_DATA);
    outb(WRITE_TO_CONFIG, KEYBOARD_CMD);
    outb(config & 0xBC, KEYBOARD_DATA); // disables interrupts

    // perform tests
    outb(TEST_DEVICE, KEYBOARD_DATA);
    unsigned char test_results = inb(KEYBOARD_DATA);
    if (test_results != 0x55) { // pass on 0x55
        printf("ERROR: keyboard failed device test.\n");
        return -1;
    }
    outb(TEST_PORT1, KEYBOARD_CMD);
    test_results = inb(KEYBOARD_DATA);
    if (test_results) { // pass on 0x00
        printf("ERROR: keyboard failed port test.\n");
        return -1;
    }

    // enable device
    outb(ENABLE_PORT1, KEYBOARD_CMD);
    outb(READ_FROM_CONFIG, KEYBOARD_DATA);
    config = inb(KEYBOARD_DATA);
    outb(WRITE_TO_CONFIG, KEYBOARD_CMD);
    outb(config | 0x01, KEYBOARD_DATA); // reenable interrupts

    // wait for input buffer to be empty (or time out)
    unsigned char status;
    do {
        status = inb(KEYBOARD_STATUS);
    } while ((status & 0x02) && !(status & 0x40));
    if (status & 0x40) { // check if we timed out
        printf("ERROR: keyboard input buffer timed out.\n");
        return -1;
    }

    // send reset byte
    outb(0xFF, KEYBOARD_DATA);

    return 0;
}
