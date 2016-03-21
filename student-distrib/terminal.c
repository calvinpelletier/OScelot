// terminal.c

#include "terminal.h"
#include "lib.h"
#include "i8259.h"


/* Local variables by group OScelot */
static uint8_t caps_active = 0;
static uint8_t ctrl_active = 0;
static uint8_t shift_active = 0;
static int8_t buf_pos = 0;

int8_t kbd_is_read = 0;
int8_t terminal_buffer[BUFFER_SIZE];
int8_t terminal_rd[BUFFER_SIZE];
uint32_t cur_buf_pos = 0;
uint32_t cur_buf_size = 0;
pos_t buf_start;


/* Function Declarations */
void keyboardHandler(void);
void do_self(unsigned char scancode, pos_t cur_position);
void do_spec(unsigned char scancode);

static void _do_key_press(unsigned char scanword, unsigned char chars[], pos_t cur_position);
static void _update_buf_pos(pos_t cur_position);
static void _print_to_terminal(int8_t buf_pos);

/*
 * keyboardHandler
 *   DESCRIPTION:  Handler for keyboard interrupts
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes to the terminal buffer
 */
void keyboardHandler(void) {
    unsigned char scancode;
    unsigned char key_released_code;
    pos_t cur_position;

    disable_irq(KEYBOARD_IRQ_NUM);

    scancode = inb(KEYBOARD_DATA);
    key_released_code = scancode | 0x80;

    cur_position = get_pos();

    switch (scancode) {
        case BACKSPACE:
            do_spec(BACKSPACE);
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
            do_spec(ENTER);
            break;
        case LEFT_SHIFT:
            shift_active = 1;
            break;
    }

    if (scancode == key_released_code) {
        // TODO: write special key actions
    }

    // TODO: write CTRL-L actions and normal character actions
    if (cur_buf_size < BUFFER_SIZE - 2) {
        do_self(scancode, cur_position);
    }

    set_cursor(cur_buf_pos - cur_buf_size);

    send_eoi(KEYBOARD_IRQ_NUM);
    enable_irq(KEYBOARD_IRQ_NUM);
}

/*
 * do_self
 *   DESCRIPTION:  Helper function that handles regular keys (lowercase 
 *                 characters).
 *   INPUTS:       scancode     - scancode of key that has been pressed
 *                 cur_position - current position on the screen
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the terminal buffer
 */
void do_self(unsigned char scancode, pos_t cur_position) {
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

void do_spec(unsigned char scancode) {
    int i;

    if (scancode == ENTER) {
        terminal_buffer[cur_buf_pos] = '\n';

        _print_to_terminal(buf_pos);

        strncpy(terminal_rd, terminal_buffer, BUFFER_SIZE);
        buf_clear();

        kbd_is_read = 1;
        buf_pos = 0;
    } else {
        if (cur_buf_pos > 0) {
            pos_t prev_pos = get_pos();

            cur_buf_pos--;
            cur_buf_size--;

            if (cur_buf_pos == (NUM_COLS - 1)) {
                prev_pos.pos_x = NUM_COLS - 1;
                prev_pos.pos_y--;
                
                buf_start.pos_y--;
                
                buf_pos = 0;
            }

            for (i = cur_buf_size + 1; i > cur_buf_pos; i--) {
                terminal_buffer[i - 1] = terminal_buffer[i];
            }

            _print_to_terminal(buf_pos);
            putc(' ');

            if (prev_pos.pos_x <= 0) {
                set_pos(NUM_COLS - 1, prev_pos.pos_y - 1);
            } else {
                set_pos(prev_pos.pos_x - 1, prev_pos.pos_y);
            }
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

static void _update_buf_pos(pos_t cur_position) {
    if (cur_position.pos_x >= NUM_COLS - 1) {
        buf_start.pos_x = 0;
        buf_start.pos_y++;
        
        set_pos(buf_start.pos_x, buf_start.pos_y);
        buf_pos = NUM_COLS;
    } else {
        set_pos(cur_position.pos_x + 1, cur_position.pos_y);
    }
}

static void _print_to_terminal(int8_t buf_pos) {
    set_pos(buf_start.pos_x, buf_start.pos_y);
    puts(terminal_buffer + buf_pos);
}

int32_t terminal_write(int32_t fd, const char* buf, int32_t nbytes) {
    int i;
    int num_bytes = 0;

    if (buf != NULL) {
        for (i = 0; i < nbytes; i++) {
            putc(buf[i]);

            buf_start.pos_x++;

            if (buf[i] == '\n' || buf_start.pos_x == NUM_COLS) {
                buf_start.pos_x = 0;
                buf_start.pos_y++;

                if (buf_start.pos_y >= NUM_ROWS) {
                    buf_start.pos_y = NUM_ROWS - 1;
                }
            }

            num_bytes++;
        }
    } else {
        return -1;
    }

    return num_bytes++;
}

int32_t terminal_read(int32_t fd, char* buf, int32_t nbytes) {
    int i;
    int num_bytes = 0;

    while (kbd_is_read == 0) {
        sti();
    }

    while (i < nbytes && terminal_rd[i] != NULL) {
        buf[i] = terminal_rd[i];
        num_bytes++;
        i++;
    }

    buf_clear();

    kbd_is_read = 0;

    return num_bytes;
}

void buf_clear(void) {
    int i;

    for (i = 0; i < BUFFER_SIZE; i++) {
        terminal_rd[i] = NULL;
        terminal_buffer[i] = NULL;
    }

    cur_buf_size = 0;
    cur_buf_pos = 0;
}
