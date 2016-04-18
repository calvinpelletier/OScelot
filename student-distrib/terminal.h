// keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "lib.h"
#include "filesys.h"

/* Custom defines added by group OScelot */
#define KEYBOARD_DATA 0x60
#define BUFFER_SIZE   128
#define KEYBOARD_MASK 0x80

#define BACKSPACE   0x0E
#define CAPS_LOCK   0x3A
#define CTRL        0x1D
#define ENTER       0x1C
#define RIGHT_SHIFT 0x36
#define SPACE       0x39
#define LEFT_SHIFT  0x2A
#define L           0x26
#define C 			0x2E
#define LEFT_ARROW  0x4B
#define RIGHT_ARROW 0x4D
#define ALT         0x38
#define F1          0x3B
#define F2          0x3C
#define F3          0x3D

#define SHELL_PROMPT_OFFSET 7

/* Function Declarations */
void keyboardHandler();
void do_self(uint8_t  scancode, pos_t cur_position);
void do_spec(uint8_t  scancode);
void do_caps(uint8_t  scancode, pos_t cur_position);
void do_shift(uint8_t  scancode, pos_t cur_position);
void do_shiftcap(uint8_t  scancode, pos_t cur_position);
void buf_clear();
void t_buf_clear(void);


extern int32_t terminal_write(file_t * file, uint8_t  * buf, int32_t nbytes);
extern int32_t terminal_read(file_t * file, uint8_t * buf, int32_t nbytes);
extern int32_t terminal_open();
extern int32_t terminal_close(file_t * file);


#endif
