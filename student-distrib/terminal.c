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

/* Local functions by group OScelot */
void do_reg(uint8_t scancode);
void do_spec(uint8_t scancode);
static void _do_key_press(uint8_t character);

void terminal_init(int num) {
    terminal[num].kbd_is_read = 0;
    terminal[num].t_buf_offset = 0;
    terminal[num].cur_buf_pos = 0;
    terminal[num].buf_start.x = 0;
    terminal[num].buf_start.y = 0;
    terminal[num].shell_offset = 0;
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
        cli();
        send_eoi(KEYBOARD_IRQ_NUM);
        enable_irq(KEYBOARD_IRQ_NUM);
        terminal_switch(0);
        return;
    }
    if (alt_active && scancode == F2) {
        cli();
        send_eoi(KEYBOARD_IRQ_NUM);
        enable_irq(KEYBOARD_IRQ_NUM);
        terminal_switch(1);
        return;
    }
    if (alt_active && scancode == F3) {
        cli();
        send_eoi(KEYBOARD_IRQ_NUM);
        enable_irq(KEYBOARD_IRQ_NUM);
        terminal_switch(2);
        return;
    }

    if (terminal[cur_terminal].cur_buf_pos < BUFFER_SIZE - 1) {
        do_reg(scancode);
    }

    /* Move cursor to the right spot */
    set_cursor(0);

    set_video_context(processes[CPID].terminal);

    /* Send EOI and enable the keyboard IRQ again so we keep getting keys */
    send_eoi(KEYBOARD_IRQ_NUM);
    enable_irq(KEYBOARD_IRQ_NUM);
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
    terminal[old_terminal].buf_start = get_pos();
    set_pos(terminal[cur_terminal].buf_start.x, terminal[cur_terminal].buf_start.y);

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

    save_video_context(old_terminal);
    load_video_context(cur_terminal);
    if (processes[CPID].terminal == cur_terminal) {
        set_video_context(ACTIVE_CONTEXT);
    } else {
        set_video_context(processes[CPID].terminal);
    }
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

    /* Without this conditional statement, it prints random characters intermittently */
    if (scancode <= SPACE) {
        if (self_chars[scancode] != NULL) {
            _do_key_press(self_chars[scancode]);
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
void do_spec(uint8_t scancode) {
    int32_t  i;  /* Loop counter */

    /* Depending on the scancode, do the special action for each key */
    switch (scancode) {
        case ENTER:
            /* Append a newline to the keyboard buffer */
            terminal[cur_terminal].keyboard_buffer[terminal[cur_terminal].cur_buf_pos] = '\n';

            putc('\n');

            /* Set kbd_is_read flag so we know it can be read */
            terminal[cur_terminal].kbd_is_read = 1;

            break;
        case BACKSPACE:
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
 *   SIDE EFFECTS: Overwrites the terminal buffer and system call buffer
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
 *   SIDE EFFECTS: Overwrites the terminal buffer and keyboard buffer
 */
int32_t terminal_read(file_t * file, uint8_t * buf, int32_t nbytes) {
    int32_t  i = 0;          /* Loop counter         */
    int32_t  num_bytes = 0;  /* Number of bytes read */

    /* Spin until ENTER is pressed */
    while (terminal[processes[CPID].terminal].kbd_is_read == 0) {
        sti();
    }

    /* Whatever is in the terminal_buffer buffer goes into the input buffer */
    while (i < nbytes && terminal[processes[CPID].terminal].keyboard_buffer[i] != NULL) {
        buf[i] = terminal[processes[CPID].terminal].keyboard_buffer[i];
        num_bytes++;
        i++;
    }

    /* Turn kbd_is_read flag to accept more interrupts */
    terminal[processes[CPID].terminal].kbd_is_read = 0;

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
static void _do_key_press(uint8_t character) {
    /* Place the character into the keyboard buffer */
    terminal[cur_terminal].keyboard_buffer[terminal[cur_terminal].cur_buf_pos] = character;

    terminal[cur_terminal].cur_buf_pos++;

    putc(character);
}
