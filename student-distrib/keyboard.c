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
    outb(KEYBOARD_CMD, DISABLE_PORT1);
    outb(KEYBOARD_CMD, DISABLE_PORT2); // will be ignored if there is no port 2
    inb(KEYBOARD_DATA); // flushes output buffer

    // set config
    outb(KEYBOARD_DATA, READ_FROM_CONFIG);
    unsigned char config = inb(KEYBOARD_DATA);
    outb(KEYBOARD_CMD, WRITE_TO_CONFIG);
    outb(KEYBOARD_DATA, config & 0xBC); // disables interrupts

    // perform tests
    outb(KEYBOARD_DATA, TEST_DEVICE);
    unsigned char test_results = inb(KEYBOARD_DATA);
    if (test_results != 0x55) { // pass on 0x55
        return -1;
    }
    outb(KEYBOARD_CMD, TEST_PORT1);
    test_results = inb(KEYBOARD_DATA);
    if (test_results) { // pass on 0x00
        return -1;
    }

    // enable device
    outb(KEYBOARD_CMD, ENABLE_PORT1);
    outb(KEYBOARD_DATA, READ_FROM_CONFIG);
    unsigned char config = inb(KEYBOARD_DATA);
    outb(KEYBOARD_CMD, WRITE_TO_CONFIG);
    outb(KEYBOARD_DATA, config | 0x01); // reenable interrupts

    // wait for input buffer to be empty (or time out)
    unsigned char status;
    do {
        status = inb(KEYBOARD_STATUS);
    } while ((status & 0x02) && !(status & 0x40));
    if (status & 0x40) { // check if we timed out
        return -1;
    }

    // send reset byte
    outb(KEYBOARD_DATA, 0xFF);

    return 0;
}
