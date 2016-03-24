// keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "lib.h"

/* Custom defines added by group OScelot */
#define KEYBOARD_DATA 0x60
#define BUFFER_SIZE   128
#define MAX_BUF_SIZE  256
#define KEYBOARD_MASK 0x80

#define BACKSPACE   0x0E
#define CAPS_LOCK   0x3A
#define CTRL        0x1D
#define ENTER       0x1C
#define RIGHT_SHIFT 0x36
#define SPACE       0x39
#define LEFT_SHIFT  0x2A
#define L           0x26

/* Function Declarations */
void keyboardHandler(void);
void do_self(unsigned char scancode, pos_t cur_position);
void do_spec(unsigned char scancode);
void do_caps(unsigned char scancode, pos_t cur_position);
void do_shift(unsigned char scancode, pos_t cur_position);
void do_shiftcap(unsigned char scancode, pos_t cur_position);
void buf_clear(void);

int32_t terminal_write(int32_t fd, const char * buf, int32_t nbytes);
int32_t terminal_read(int32_t fd, char* buf, int32_t nbytes);
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);


#endif
