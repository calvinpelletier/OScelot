// keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

/* Custom defines added by group OScelot */
#define KEYBOARD_DATA 0x60
#define BUFFER_SIZE   128

#define BACKSPACE   0x0E
#define CAPS_LOCK   0x3A
#define CTRL        0x1D
#define ENTER       0x1C
#define SPACE       0x39
#define LEFT_SHIFT  0x2A
#define L           0x26


extern void keyboardHandler(void);

#endif
