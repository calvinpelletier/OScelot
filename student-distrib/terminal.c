// terminal.c

#include "terminal.h"
#include "lib.h"
#include "i8259.h"


/* Local variables by group OScelot */
static uint8_t caps_active = 0;
static uint8_t ctrl_active = 0;
static uint8_t shift_active = 0;
static uint8_t buf_pos = 0;

uint8_t terminal_buffer[BUFFER_SIZE];


/* Function Declarations */
void keyboardHandler(void);
void do_self(void);

/*
keyboardHandler
    DESCRIPTION: called on keyboard interrupts
    INPUT: none
    OUTPUT: none
    RETURNS: none
*/
void keyboardHandler(void) {
    // printf("~~~KEYBOARD~~~\n");
    // while (inb(0x64) & 0x01) {
    //     printf("%x\n", inb(KEYBOARD_DATA));
    // }
    // printf("~~~~~~~~~~~~~~\n");
    // scroll();
    // send_eoi(KEYBOARD_IRQ_NUM);

    unsigned char scancode;
    unsigned char key_released_code;

    scancode = inb(KEYBOARD_DATA);
    key_released_code = scancode | 0x80;

    switch (scancode) {
        case BACKSPACE:
            // TODO: write backspace function
            break;
        case CAPS_LOCK:
            /* Toggles caps lock on and off */
            if (caps_active) {
                caps_active = 0;
            } else {
                caps_active = 1;
            }

            break;
        case CTRL:
            ctrl_active = 1;
            break;
        case ENTER:
            // TODO: write enter function
            break;
        case LEFT_SHIFT:
            shift_active = 1;
            break;
    }

    if (scancode == key_released_code) {
        // TODO: write special key actions
    }

    // TODO: write CTRL-L actions and normal character actions
}

void do_self(void) {
    // TODO: write this function

}

