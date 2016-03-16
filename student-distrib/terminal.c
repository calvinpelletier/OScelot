// terminal.c

#include "terminal.h"
#include "lib.h"
#include "i8259.h"


/* Local variables by group OScelot */
static uint8_t caps = 0;
static uint8_t ctrl = 0;
static uint8_t shift = 0;
static uint8_t buf_pos = 0;

// FUNCTION DECLARATIONS
void keyboardHandler(void);

// GLOBAL FUNCTIONS

/*
keyboardHandler
    DESCRIPTION: called on keyboard interrupts
    INPUT: none
    OUTPUT: none
    RETURNS: none
*/
void keyboardHandler(void) {
    // printf("~~~KEYBOARD~~~\n");
    // while (inb(KEYBOARD_STATUS) & 0x01) {
    //     printf("%x\n", inb(KEYBOARD_DATA));
    // }
    // printf("~~~~~~~~~~~~~~\n");
    // send_eoi(KEYBOARD_IRQ_NUM);

    unsigned char scancode;
    unsigned char key_released_code;

    scancode = inb(KEYBOARD_DATA);
    key_released_code = scancode | 0x80;
}

