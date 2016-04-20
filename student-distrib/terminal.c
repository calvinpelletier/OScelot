/* terminal.c - Handles keyboard presses and prints to terminal. */

#include "terminal.h"
#include "i8259.h"
#include "syscalls.h"


/* Local variables by group OScelot */
static int caps_active = 0;              // Booleans for special keys
static int ctrl_active = 0;
static int shift_active = 0;
static int alt_active = 0;

terminal_t terminal[NUM_TERMINALS];
int cur_terminal = 0;
terminal_t* t; // shortcut for terminal[cur_terminal]

/* Local functions by group OScelot */
void terminal_switch(int new_terminal);
void do_reg(uint8_t scancode);
void do_spec(uint8_t scancode);

void terminal_init(int num) {
    terminal[num].kbd_is_read = 0;
    terminal[num].buf_pos = 0;
    terminal[num].pos.x = 0;
    terminal[num].pos.y = 0;
}

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

    disable_irq(KEYBOARD_IRQ_NUM);

    cli();

    // set shortcut t
    t = &terminal[cur_terminal];

    // write to the visible video memory
    set_video_context(ACTIVE_CONTEXT);

    /* Receive data from the keyboard */
    scancode = inb(KEYBOARD_DATA);

    /* Calculate the release code by OR'ing with 0x80 */
    key_released_code = scancode | KEYBOARD_MASK;

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
        case ALT:
            alt_active = 1;
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
        } else if (alt_active && ((scancode & ~(KEYBOARD_MASK)) == ALT)) {
            alt_active = 0;
        }
    }

    // handles special key combo of ALT-F1/F2/F3
    if (alt_active && scancode == F1) {
        // doesn't need an error check
        send_eoi(KEYBOARD_IRQ_NUM);
        enable_irq(KEYBOARD_IRQ_NUM);
        terminal_switch(0);
        return;
    }
    if (alt_active && scancode == F2) {
        // error check
        int process_available = 0;
        int i;
        for (i = 1; i < MAX_PROCESSES + 1; i++) {
            if (!processes[i].running) {
                process_available = 1;
            }
        }
        if (active_processes[1] != 0 || process_available) {
            send_eoi(KEYBOARD_IRQ_NUM);
            enable_irq(KEYBOARD_IRQ_NUM);
            terminal_switch(1);
            return;
        }
    }
    if (alt_active && scancode == F3) {
        // error check
        int process_available = 0;
        int i;
        for (i = 1; i < MAX_PROCESSES + 1; i++) {
            if (!processes[i].running) {
                process_available = 1;
            }
        }
        if (active_processes[2] != 0 || process_available) {
            send_eoi(KEYBOARD_IRQ_NUM);
            enable_irq(KEYBOARD_IRQ_NUM);
            terminal_switch(2);
            return;
        }
    }

    if (ctrl_active && scancode == C) {
        clear();

        /* Reset buffer position to (0, 0) */
        set_pos(0, 0);
        puts("391OS> ");

        /* Clear the whole buffer */
        t->buf_pos = 0;

        // so that the process in the current terminal is halted next time it receives processor time
        needs_to_be_halted[cur_terminal] = 1;

    /* Handles the special key combo of CTRL-L which
     * clears the screen except for the terminal buffer.
     */
    } else if (ctrl_active && scancode == L) {
        clear();

        /* Reset buffer position to (0, 0) */
        set_pos(0, 0);
        puts("391OS> ");

        /* Clear the whole keyboard buffer */
        t->buf_pos = 0;

    } else if (t->buf_pos < BUFFER_SIZE - 1) {
        do_reg(scancode);
    }

    /* Move cursor to the right spot */
    set_cursor(0);

    // adjust video memory back to what it was
    if (processes[CPID].terminal == cur_terminal) {
        set_video_context(ACTIVE_CONTEXT);
    } else {
        set_video_context(processes[CPID].terminal);
    }

    /* Send EOI and enable the keyboard IRQ again so we keep getting keys */
    send_eoi(KEYBOARD_IRQ_NUM);
    enable_irq(KEYBOARD_IRQ_NUM);
    sti();
}

/*
 * terminal_switch
 *   DESCRIPTION:  switches to a new terminal
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: writes to video memory
 */
void terminal_switch(int new_terminal) {
    if (new_terminal == cur_terminal) {
        return;
    }

    int old_terminal = cur_terminal;
    cur_terminal = new_terminal;

    // save anything we need, restore new cursor
    terminal[old_terminal].pos = get_pos();
    set_pos(terminal[new_terminal].pos.x, terminal[new_terminal].pos.y);
    set_cursor(0);

    // check if we need to load the base shell
    if (active_processes[cur_terminal] == 0) {
        save_video_context(old_terminal);
        set_video_context(ACTIVE_CONTEXT);
        clear();
        int old_esp, old_ebp;
        __asm__("movl %%esp, %0; movl %%ebp, %1"
                 :"=g"(old_esp), "=g"(old_ebp) /* outputs */
                );
        processes[CPID].esp_switch = old_esp;
        processes[CPID].ebp_switch = old_ebp;
        execute_base_shell(cur_terminal);
        return;
    }

    // adjust video memory
    save_video_context(old_terminal);
    load_video_context(cur_terminal);
    if (processes[CPID].terminal == cur_terminal) {
        set_video_context(ACTIVE_CONTEXT);
    } else {
        set_video_context(processes[CPID].terminal);
    }
}

/*
 * do_reg
 *   DESCRIPTION:  Helper function that handles regular keys
 *   INPUTS:       scancode     - scancode of key that has been pressed
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 */
void do_reg(uint8_t scancode) {
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
        char character;
        if (shift_active && caps_active) {
            character = combo_chars[scancode];
        } else if (shift_active) {
            character = shift_chars[scancode];
        } else if (caps_active) {
            character = caps_chars[scancode];
        } else {
            character = self_chars[scancode];
        }

        if (character == NULL) {
            return;
        }

        t->buffer[t->buf_pos] = character;
        t->buf_pos++;
        putc(character);
    }
}

/*
 * do_spec
 *   DESCRIPTION:  Helper function that handles special keys: ENTER,
 *                 BACKSPACE, and CAPS LOCK.
 *   INPUTS:       scancode - scancode of key that has been pressed
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 */
void do_spec(uint8_t scancode) {
    int i;  /* Loop counter */

    /* Depending on the scancode, do the special action for each key */
    switch (scancode) {
        case ENTER:
            /* Append a newline to the keyboard buffer */
            t->buffer[t->buf_pos] = '\n';
            putc('\n');
            /* Set kbd_is_read flag so we know it can be read */
            t->kbd_is_read = 1;
        break;

        case BACKSPACE:
            /* If we're not at the beginning of the buffer, we can delete */
            if (t->buf_pos == 0) {
                return;
            }
            t->buf_pos--;
            pos_t old_pos = get_pos();
            pos_t new_pos;
            if (old_pos.x == 0) {
                new_pos.x = NUM_COLS - 1;
                new_pos.y = old_pos.y - 1;
            } else {
                new_pos.x = old_pos.x - 1;
                new_pos.y = old_pos.y;
            }
            set_pos(new_pos.x, new_pos.y);
            putc(' ');
            set_pos(new_pos.x, new_pos.y);
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
 * terminal_write
 *   DESCRIPTION:  System call that writes the terminal buffer to the screen.
 *   INPUTS:       file   - not used
 *                 buf    - buffer from which to write data from
 *                 nbytes - number of bytes to write
 *   OUTPUTS:      none
 *   RETURN VALUE: Number of bytes written
 */
int32_t terminal_write(file_t * file, uint8_t * buf, int32_t nbytes) {
    int32_t  i;              /* Loop counter                           */
    int32_t  num_bytes = 0;  /* Number of bytes that have been written */

    if (buf != NULL) {
        /* Print each character in the passed in buffer to the screen */
        for (i = 0; i < nbytes; i++) {
            putc(buf[i]);
            num_bytes++;
        }
    } else {
        return -1;
    }
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
 */
int32_t terminal_read(file_t * file, uint8_t * buf, int32_t nbytes) {
    int32_t  i = 0;          /* Loop counter         */
    int32_t  num_bytes = 0;  /* Number of bytes read */

    /* Spin until ENTER is pressed */
    while (terminal[processes[CPID].terminal].kbd_is_read == 0) {
        sti();
    }

    /* Whatever is in the terminal buffer goes into the input buffer */
    while (i < nbytes && i < terminal[processes[CPID].terminal].buf_pos) {
        buf[i] = terminal[processes[CPID].terminal].buffer[i];
        num_bytes++;
        i++;
    }

    // clear buffer
    terminal[processes[CPID].terminal].buf_pos = 0;

    /* Turn kbd_is_read flag to accept more interrupts */
    terminal[processes[CPID].terminal].kbd_is_read = 0;

    return num_bytes;
}

/*
 * terminal_open
 *   DESCRIPTION:  System call that opens the filename. Not used by terminal.
 *   INPUTS:       filename - name of file to open
 *   OUTPUTS:      none
 *   RETURN VALUE: -1
 */
int32_t terminal_open() {
    return -1;
}

/*
 * terminal_close
 *   DESCRIPTION:  System call that closes a file. Not used by terminal.
 *   INPUTS:       file- file descriptor ptr of file to close
 *   OUTPUTS:      none
 *   RETURN VALUE: -1
 */
int32_t terminal_close(file_t * file) {
    return -1;
}
