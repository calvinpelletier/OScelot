// terminal.c

#include "terminal.h"
#include "lib.h"
#include "i8259.h"


/* Local variables by group OScelot */
static uint8_t caps_active = 0;
static uint8_t ctrl_active = 0;
static uint8_t shift_active = 0;
static int8_t buf_pos = 0;

int8_t terminal_buffer[BUFFER_SIZE];
uint32_t cur_buf_pos = 0;
uint32_t cur_buf_size = 0;
pos_t buf_start;


/* Function Declarations */
void keyboardHandler(void);
void do_self(unsigned char scancode, pos_t cur_position);

static void _do_key_press(unsigned char scanword, unsigned char chars[], pos_t cur_position);
static void _update_buf_pos(pos_t cur_position);
static void _print_to_terminal(int8_t buf_pos);

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
    pos_t cur_position;

    disable_irq(KEYBOARD_IRQ_NUM);

    scancode = inb(KEYBOARD_DATA);
    key_released_code = scancode | 0x80;

    cur_position = get_pos();

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
    do_self(scancode, cur_position);

    send_eoi(KEYBOARD_IRQ_NUM);
    enable_irq(KEYBOARD_IRQ_NUM);
}

void do_self(unsigned char scancode, pos_t cur_position) {
    // TODO: write this function
    unsigned char self_chars[64] = {
        0, 0, '1', '2', '3', '4', '5', '6', 
        '7', '8', '9', '0', '-', '=', 0, 0, 
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
        'o', 'p','[', ']', 0, 0, 'a', 's', 
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
        '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 
        'b', 'n', 'm', ',', '.', '/', 0, 0, 
        0,' ',0, 0, 0, 0, 0, 0
    };

    if (scancode <= SPACE) {
        if (cur_buf_size == 0) {
            buf_start = cur_position;
        }

        if (self_chars[scancode] != NULL) {
            _do_key_press(scancode, self_chars, cur_position);
        }
    }
}

static void _do_key_press(unsigned char scancode, unsigned char chars[], pos_t cur_position) {
    int i;

    for (i = cur_buf_size; i > cur_buf_pos; i--) {
        terminal_buffer[i] = terminal_buffer[i - 1];
    }

    terminal_buffer[cur_buf_pos] = chars[scancode];
    cur_buf_size++;
    cur_buf_pos++;

    _print_to_terminal(buf_pos);
    _update_buf_pos(cur_position);
}

void _update_buf_pos(pos_t cur_position) {
    if (cur_position.pos_x >= NUM_COLS - 1) {
        buf_start.pos_x = 0;
        buf_start.pos_y++;
        
        set_pos(buf_start.pos_x, buf_start.pos_y);
        buf_pos = NUM_COLS - 9;
    } else {
        set_pos(cur_position.pos_x + 1, cur_position.pos_y);
    }
}

void _print_to_terminal(int8_t buf_pos) {
    set_pos(buf_start.pos_x, buf_start.pos_y);
    puts(terminal_buffer + buf_pos);
}
