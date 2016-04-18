/* terminal.c - Handles keyboard presses and prints to terminal. */

#include "terminal.h"
#include "i8259.h"
#include "syscalls.h"


/* Local variables by group OScelot */
static uint8_t caps_active = 0;              // Booleans for special keys
static uint8_t ctrl_active = 0;
static uint8_t shift_active = 0;
static uint8_t t_buf_offset = 0;             /* Offset to determine from where in the terminal 
                                              * to start printing from
                                              */

static int8_t kbd_is_read = 0;               // Boolean to determine if the keyboard has been read
static int8_t keyboard_buffer[BUFFER_SIZE];  // Keyboard buffer
static int8_t terminal_buffer[BUFFER_SIZE];  // System call terminal buffer
static uint32_t cur_buf_pos = 0;             // Current buffer position
static pos_t buf_start;                      // pos_t struct to hold the coordinates of the buffer
static int shell_offset = 0;                     /* Keeps track of how much user has written to beginning of line in terminal
                                              * for handling key presses afterward.
                                              */
/* Local functions by group OScelot */
static void _do_key_press(uint8_t  scancode, uint8_t  chars[], pos_t cur_position);
static void _update_buf_pos(pos_t cur_position);
static void _print_to_terminal(uint8_t t_buf_offset);

/*
 * keyboardHandler
 *   DESCRIPTION:  Handler for keyboard interrupts
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes to the terminal buffer
 */
void keyboardHandler(void) {
    uint8_t  scancode;
    uint8_t  key_released_code;
    pos_t cur_position;

    disable_irq(KEYBOARD_IRQ_NUM);

    /* Receive data from the keyboard */
    scancode = inb(KEYBOARD_DATA);

    /* Calculate the release code by OR'ing with 0x80 */
    key_released_code = scancode | KEYBOARD_MASK;

    /* Get the current position in the terminal */
    cur_position = get_pos();

    /* Check for special keys and do the appropriate function call */
    switch (scancode) {
        case BACKSPACE:
            do_spec(BACKSPACE);
            break;
        case CAPS_LOCK:
            do_spec(CAPS_LOCK);
            break;
        case CTRL:
            ctrl_active = 1;
            break;
        case ENTER:
            do_spec(ENTER);
            break;
        case RIGHT_SHIFT:
            shift_active = 1;
            break;
        case LEFT_SHIFT:
            shift_active = 1;
            break;
        default:
            break;
    }

    /* Since CTRL and SHIFT are not toggled keys, we need to turn them off 
     * if they are released. If they are held down, we need to use a different
     * character array.
     */
    if (scancode == key_released_code) {
        if (ctrl_active && ((scancode & ~(KEYBOARD_MASK)) == CTRL)) {
            ctrl_active = 0;
        } else if (shift_active && ((scancode & ~(KEYBOARD_MASK)) == LEFT_SHIFT || 
                   (scancode & ~(KEYBOARD_MASK)) == RIGHT_SHIFT)) {
            shift_active = 0;
        }
    }

    /* Handles the special key combo of CTRL-C which 
     * halts the currently running program.
     */
    if (ctrl_active && scancode == C) {
        clear();
        
        /* Reset buffer position to (0, 0) */
        set_pos(0, 0);
        puts("391OS> ");

        shell_offset = SHELL_PROMPT_OFFSET;
        buf_start.pos_x = SHELL_PROMPT_OFFSET;
        buf_start.pos_y = 0;

        /* Clear the whole keyboard buffer */
        buf_clear();
        
        send_eoi(KEYBOARD_IRQ_NUM);
        enable_irq(KEYBOARD_IRQ_NUM);

        exception_halt();

    /* Handles the special key combo of CTRL-L which 
     * clears the screen except for the terminal buffer.
     */
    } else if (ctrl_active && scancode == L) {
        clear();
        
        /* Reset buffer position to (0, 0) */
        set_pos(0, 0);
        puts("391OS> ");

        shell_offset = SHELL_PROMPT_OFFSET;
        buf_start.pos_x = SHELL_PROMPT_OFFSET;
        buf_start.pos_y = 0;

        /* Clear the whole keyboard buffer */
        buf_clear();
        
    /* '\n' is 1 byte so the buffer should stop at 127 instead of 128 */
    } else if (cur_buf_pos < BUFFER_SIZE - 1) { 
        if (!caps_active && !shift_active) {
            do_self(scancode, cur_position);         
        } else if (caps_active && shift_active) {
            do_shiftcap(scancode, cur_position);
        } else if (shift_active) {
            do_shift(scancode, cur_position);
        } else {
            do_caps(scancode, cur_position);
        }
    }

    /* Move cursor to the right spot */
    set_cursor(0);

    // if (scancode == LEFT_ARROW) {
    //     set_cursor(-1);
    //     buf_offset++;
    // } else if (scancode == RIGHT_ARROW) {
    //     set_cursor(1);
    //     buf_offset--;
    // }

    /* Send EOI and enable the keyboard IRQ again so we keep getting keys */
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
void do_self(uint8_t  scancode, pos_t cur_position) {
    /* Character array using scancode set 1 */
    uint8_t  self_chars[64] = {
        0, 0, '1', '2', '3', '4', '5', '6', 
        '7', '8', '9', '0', '-', '=', 0, 0, 
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
        'o', 'p','[', ']', 0, 0, 'a', 's', 
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
        '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 
        'b', 'n', 'm', ',', '.', '/', 0, 0, 
        0,' ',0, 0, 0, 0, 0, 0
    };

    /* Without this conditional statement, it prints random characters intermittently */
    if (scancode <= SPACE) {
        if (cur_buf_pos == 0) {
            buf_start = cur_position;
        }

        if (self_chars[scancode] != NULL) {
            _do_key_press(scancode, self_chars, cur_position);
        }
    }
}

/*
 * do_caps
 *   DESCRIPTION:  Helper function that handles CAPS keys.
 *   INPUTS:       scancode     - scancode of key that has been pressed
 *                 cur_position - current position on the screen
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the terminal buffer
 */
void do_caps(uint8_t  scancode, pos_t cur_position) {
    /* CAPS character array using scancode set 1 */
    uint8_t  caps_chars[64] = {
        0, 0, '1', '2', '3', '4', '5', '6', 
        '7', '8', '9', '0', '-', '=', 0, 0, 
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 
        'O', 'P','[', ']', 0, 0, 'A', 'S', 
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
        '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', ',', '.', '/', 0, 0, 
        0,' ',0, 0, 0, 0, 0, 0
    };

    /* Without this conditional statement, it prints random characters intermittently */
    if (scancode <= SPACE) {
        if (cur_buf_pos == 0) {
            buf_start = cur_position;
        }

        if (caps_chars[scancode] != NULL) {
            _do_key_press(scancode, caps_chars, cur_position);
        }
    }
}

/*
 * do_shift
 *   DESCRIPTION:  Helper function that handles shift keys.
 *   INPUTS:       scancode     - scancode of key that has been pressed
 *                 cur_position - current position on the screen
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the terminal buffer
 */
void do_shift(uint8_t  scancode, pos_t cur_position) {
    /* SHIFT character array using scancode set 1 */
    uint8_t  shift_chars[64] = {
        0, 0, '!', '@', '#', '$', '%', '^', 
        '&', '*', '(', ')', '_', '+', 0, 0, 
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 
        'O', 'P','{', '}', 0, 0,  'A', 'S', 
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
        '\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', '<', '>', '?', 0, 0, 
        0, ' ', 0, 0, 0, 0, 0, 0
    };

    /* Without this conditional statement, it prints random characters intermittently */
    if (scancode <= SPACE) {
        if (cur_buf_pos == 0) {
            buf_start = cur_position;
        }

        if (shift_chars[scancode] != NULL) {
            _do_key_press(scancode, shift_chars, cur_position);
        }
    }
}

/*
 * do_shiftcap
 *   DESCRIPTION:  Helper function that handles combination of SHIFT and CAPS
 *   INPUTS:       scancode     - scancode of key that has been pressed
 *                 cur_position - current position on the screen
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the terminal buffer
 */
void do_shiftcap(uint8_t  scancode, pos_t cur_position) {
    /* SHIFT-CAP character array using scancode set 1 */
    uint8_t  combo_chars[64] = {
        0, 0, '!', '@', '#', '$', '%', '^', 
        '&', '*', '(', ')', '_', '+', 0, 0, 
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 
        'o', 'p','{', '}', 0, 0, 'a', 's', 
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',
        '\"', '~', 0, '|', 'z', 'x', 'c', 'v', 
        'b', 'n', 'm', '<', '>', '?', 0, 0, 
        0,' ',0, 0, 0, 0, 0, 0
    };

    /* Without this conditional statement, it prints random characters intermittently */
    if (scancode <= SPACE) {
        if (cur_buf_pos == 0) {
            buf_start = cur_position;
        }

        if (combo_chars[scancode] != NULL) {
            _do_key_press(scancode, combo_chars, cur_position);
        }
    }
}

/*
 * do_spec
 *   DESCRIPTION:  Helper function that handles special keys: ENTER,
 *                 BACKSPACE, and CAPS LOCK.
 *   INPUTS:       scancode - scancode of key that has been pressed
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the terminal buffer
 */
void do_spec(uint8_t  scancode) {
    int32_t  i;  /* Loop counter */
    
    /* Depending on the scancode, do the special action for each key */
    switch (scancode) {
        case ENTER:
            /* Append a newline to the keyboard buffer */
            keyboard_buffer[cur_buf_pos] = '\n';

            putc('\n');

            /* Copy the keyboard buffer to the terminal buffer */
            strncpy(terminal_buffer, keyboard_buffer, BUFFER_SIZE);

            /* Set kbd_is_read flag so we know it can be read */
            kbd_is_read = 1;
            buf_clear();

            break;
        case BACKSPACE:
            /* If we're not at the beginning of the buffer, we can delete */
            if (cur_buf_pos > 0) {
                pos_t prev_pos = get_pos();
                pos_t new_pos;

                /* Update the buffer position */
                cur_buf_pos--;

                /* Check if the current buffer position is at the end of the line */
                if ((cur_buf_pos + shell_offset + 1) % NUM_COLS == 0) {
                    new_pos.pos_x = NUM_COLS - 1;
                    new_pos.pos_y = prev_pos.pos_y - 1;

                    buf_start.pos_x = shell_offset;
                    buf_start.pos_y = new_pos.pos_y;

                    t_buf_offset = 0;
                } else {
                  new_pos.pos_x = prev_pos.pos_x - 1;
                  new_pos.pos_y = prev_pos.pos_y;
                }

                /* Repopulate the keyboard buffer with the appropriate characters */
                for (i = cur_buf_pos + 1; i > cur_buf_pos; i--) {
                    keyboard_buffer[i - 1] = keyboard_buffer[i];
                }

                /* Print  the buffer to the terminal plus an empty space for the deleted char */
                _print_to_terminal(t_buf_offset);
                putc(' ');

                /* Change the position to the previous position */
                set_pos(new_pos.pos_x, new_pos.pos_y);
            }

            break;
        case CAPS_LOCK:
            /* Toggle the CAPS flag so we know which character array to use */
            if (caps_active) {
                caps_active = 0;
            } else {
                caps_active = 1;
            }

            break;
        default:
            break;
    } 
}

/*
 * buf_clear
 *   DESCRIPTION:  Helper function that clears keyboard buffer.
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the keyboard buffer
 */
void buf_clear(void) {
    int32_t  i; /* Loop counter */

    /* Clear the whole keyboard buffer */
    for (i = 0; i < BUFFER_SIZE; i++) {
        keyboard_buffer[i] = NULL;
    }

    /* Reset buffer position and buffer offset */
    cur_buf_pos = 0;
    t_buf_offset = 0;
}

/*
 * t_buf_clear
 *   DESCRIPTION:  Helper function that clears terminal buffer.
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the keyboard buffer
 */
void t_buf_clear(void) {
    int32_t  i; /* Loop counter */

    /* Clear the whole keyboard buffer */
    for (i = 0; i < BUFFER_SIZE; i++) {
        terminal_buffer[i] = NULL;
    }

    /* Reset buffer position and buffer offset */
    cur_buf_pos = 0;
    t_buf_offset = 0;
}

/*
 * terminal_write
 *   DESCRIPTION:  System call that writes the terminal buffer to the screen.
 *   INPUTS:       file   - not used
 *                 buf    - buffer from which to write data from
 *                 nbytes - number of bytes to write
 *   OUTPUTS:      none
 *   RETURN VALUE: Number of bytes written
 *   SIDE EFFECTS: Overwrites the terminal buffer and system call buffer
 */
int32_t terminal_write(file_t * file, uint8_t * buf, int32_t nbytes) {
    int32_t  i;              /* Loop counter                           */
    int32_t  num_bytes = 0;  /* Number of bytes that have been written */

    if (buf != NULL) {
        /* Print each character in the passed in buffer to the screen */
        for (i = 0; i < nbytes; i++) {
          if (i == 0)
            shell_offset = get_pos().pos_x;
            putc(buf[i]);

            buf_start.pos_x++;

            /* If there is a newline character or if we're at the end of the line, wrap */
            if (buf[i] == '\n' || buf_start.pos_x == NUM_COLS) {
                buf_start.pos_x = 0;
                buf_start.pos_y++;

                /* If we're at the bottom of the screen, we're going to scroll so change pos_y */
                if (buf_start.pos_y >= NUM_ROWS) {
                    buf_start.pos_y = NUM_ROWS - 1;
                }
            }

            num_bytes++;
        }
    } else {
        return -1;
    }
    shell_offset += num_bytes;
    return num_bytes++;
}

/*
 * terminal_read
 *   DESCRIPTION:  System call that reads from the terminal buffer
 *   INPUTS:       file   - not used
 *                 buf    - buffer from which to read data to
 *                 nbytes - number of bytes to read
 *   OUTPUTS:      none
 *   RETURN VALUE: Number of bytes read
 *   SIDE EFFECTS: Overwrites the terminal buffer and keyboard buffer
 */
int32_t terminal_read(file_t * file, uint8_t * buf, int32_t nbytes) {
    int32_t  i = 0;          /* Loop counter         */
    int32_t  num_bytes = 0;  /* Number of bytes read */

    /* Spin until ENTER is pressed */
    while (kbd_is_read == 0) {
        sti();
    }

    /* Whatever is in the terminal_buffer buffer goes into the input buffer */
    while (i < nbytes && terminal_buffer[i] != NULL) {
        buf[i] = terminal_buffer[i];
        num_bytes++;
        i++;
    }
    t_buf_clear();
    
    /* Turn kbd_is_read flag to accept more interrupts */
    kbd_is_read = 0;

    return num_bytes;
}

/*
 * terminal_open
 *   DESCRIPTION:  System call that opens the filename. Not used by terminal.
 *   INPUTS:       filename - name of file to open
 *   OUTPUTS:      none
 *   RETURN VALUE: -1 if unsuccessful
 *   SIDE EFFECTS: none
 */
int32_t terminal_open() {
    return -1;
}

/*
 * terminal_close
 *   DESCRIPTION:  System call that closes a file. Not used by terminal.
 *   INPUTS:       file- file descriptor ptr of file to close
 *   OUTPUTS:      none
 *   RETURN VALUE: -1 if unsuccessful
 *   SIDE EFFECTS: none
 */
int32_t terminal_close(file_t * file) {
    return -1;
}

/*
 * _do_key_press
 *   DESCRIPTION:  Helper function that handles the actual key press and updates
 *                 the relevant variables.
 *   INPUTS:       scancode     - scancode of key that has been pressed
 *                 chars        - array of characters to use for the scancodes
 *                 cur_position - current position in the terminal buffer
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the terminal buffer and updates global variables
 */
static void _do_key_press(uint8_t  scancode, uint8_t  chars[], pos_t cur_position) {
    /* Place the character into the keyboard buffer */
    keyboard_buffer[cur_buf_pos] = chars[scancode];

    cur_buf_pos++;

    /* Print the keyboard buffer to the screen and update the position */
    _print_to_terminal(t_buf_offset);
    _update_buf_pos(cur_position);
}

/*
 * _update_buf_pos
 *   DESCRIPTION:  Helper function that to update the buffer position.
 *   INPUTS:       cur_position - current position in the terminal buffer
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Updates the buffer position global variables
 */
static void _update_buf_pos(pos_t cur_position) {
    /* If we're at the end of the line, update for scrolling and text wrapping */
    if (cur_position.pos_x >= NUM_COLS - 1) {
        buf_start.pos_x = 0;
        buf_start.pos_y++;

        set_pos(buf_start.pos_x, buf_start.pos_y);

        /* Depending on the current size of the buffer, 
         * we should copy from a different offset from
         * the beginning of the terminal buffer.
         */
        if (cur_buf_pos >= NUM_COLS * 3 - shell_offset) {
            t_buf_offset = NUM_COLS * 3 - shell_offset;
        } else if (cur_buf_pos >= NUM_COLS * 2 - shell_offset) {
            t_buf_offset = NUM_COLS * 2 - shell_offset;
        } else if (cur_buf_pos >= NUM_COLS - shell_offset) {
            t_buf_offset = NUM_COLS - shell_offset;
        } else {
            t_buf_offset = shell_offset;
        }
            
    } else {
        set_pos(cur_position.pos_x + 1, cur_position.pos_y);
    }
}

/*
 * _print_to_terminal
 *   DESCRIPTION:  Helper function that to print to the terminal buffer 
 *                 to the screen.
 *   INPUTS:       t_buf_offset - current position in the terminal buffer
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes to the terminal buffer to video memory
 */
static void _print_to_terminal(uint8_t t_buf_offset) {
    /* Update the position in the terminal so we don't overwrite anything */
    set_pos(buf_start.pos_x, buf_start.pos_y);

    /* Print to the screen with the t_buf_offset offset so we 
     * don't copy multiple lines when we're calling scroll().
     */
    puts(keyboard_buffer + t_buf_offset);
}
