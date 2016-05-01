/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#include "syscalls.h"
#include "terminal.h"

static int screen_x[NUM_TERMINALS];
static int screen_y[NUM_TERMINALS];
char* video_mem = (char *)VIDEO;

char cur_attribute = 0x05; /* Initialize current attribute to magenta */

int get_screen_xy_idx(void) {
    if (video_mem == (char*)VIDEO_0) {
        return 0;
    } else if (video_mem == (char*)VIDEO_1) {
        return 1;
    } else if (video_mem == (char*)VIDEO_2) {
        return 2;
    } else {
        return cur_terminal;
    }
}

/*
 * set_video_context
 *   DESCRIPTION:  sets which video memory get written to in library functions
 *   INPUTS:       -1 for visible memory, 0-2 for hidden (terminals 0-2)
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 */
void set_video_context(int context) {
    if (context == ACTIVE_CONTEXT) {
        video_mem = (char*)VIDEO;
    } else if (context == 0) {
        video_mem = (char*)VIDEO_0;
    } else if (context == 1) {
        video_mem = (char*)VIDEO_1;
    } else if (context == 2) {
        video_mem = (char*)VIDEO_2;
    }
}

/*
 * save_video_context
 *   DESCRIPTION:  saves visible memory in hidden memory
 *   INPUTS:       destination (0-2)
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 */
void save_video_context(int context) {
    if (context == 0) {
        memcpy((char*)VIDEO_0, (char*)VIDEO, VIDEO_SIZE);
    } else if (context == 1) {
        memcpy((char*)VIDEO_1, (char*)VIDEO, VIDEO_SIZE);
    } else if (context == 2) {
        memcpy((char*)VIDEO_2, (char*)VIDEO, VIDEO_SIZE);
    }
}

/*
 * load_video_context
 *   DESCRIPTION:  loads hidden memory into visible memory
 *   INPUTS:       source (0-2)
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 */
void load_video_context(int context) {
    if (context == 0) {
        memcpy((char*)VIDEO, (char*)VIDEO_0, VIDEO_SIZE);
    } else if (context == 1) {
        memcpy((char*)VIDEO, (char*)VIDEO_1, VIDEO_SIZE);
    } else if (context == 2) {
        memcpy((char*)VIDEO, (char*)VIDEO_2, VIDEO_SIZE);
    }
}

/*
* void clear(void);
*   Inputs: void
*   Return Value: none
*	Function: Clears video memory
*/
void
clear(void)
{
    int32_t i;
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = cur_attribute;
    }
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output.
 * */
int32_t
printf(int8_t *format, ...)
{
	/* Pointer to the format string */
	int8_t* buf = format;

	/* Stack pointer for the other parameters */
	int32_t* esp = (void *)&format;
	esp++;

	while(*buf != '\0') {
		switch(*buf) {
			case '%':
				{
					int32_t alternate = 0;
					buf++;

format_char_switch:
					/* Conversion specifiers */
					switch(*buf) {
						/* Print a literal '%' character */
						case '%':
							putc('%');
							break;

						/* Use alternate formatting */
						case '#':
							alternate = 1;
							buf++;
							/* Yes, I know gotos are bad.  This is the
							 * most elegant and general way to do this,
							 * IMHO. */
							goto format_char_switch;

						/* Print a number in hexadecimal form */
						case 'x':
							{
								int8_t conv_buf[64];
								if(alternate == 0) {
									itoa(*((uint32_t *)esp), conv_buf, 16);
									puts(conv_buf);
								} else {
									int32_t starting_index;
									int32_t i;
									itoa(*((uint32_t *)esp), &conv_buf[8], 16);
									i = starting_index = strlen(&conv_buf[8]);
									while(i < 8) {
										conv_buf[i] = '0';
										i++;
									}
									puts(&conv_buf[starting_index]);
								}
								esp++;
							}
							break;

						/* Print a number in unsigned int form */
						case 'u':
							{
								int8_t conv_buf[36];
								itoa(*((uint32_t *)esp), conv_buf, 10);
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a number in signed int form */
						case 'd':
							{
								int8_t conv_buf[36];
								int32_t value = *((int32_t *)esp);
								if(value < 0) {
									conv_buf[0] = '-';
									itoa(-value, &conv_buf[1], 10);
								} else {
									itoa(value, conv_buf, 10);
								}
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a single character */
						case 'c':
							putc( (uint8_t) *((int32_t *)esp) );
							esp++;
							break;

						/* Print a NULL-terminated string */
						case 's':
							puts( *((int8_t **)esp) );
							esp++;
							break;

						default:
							break;
					}

				}
				break;

			default:
				putc(*buf);
				break;
		}
		buf++;
	}

	/* Set cursor properly after printf to screen */
	set_cursor(0);

	return (buf - format);
}

/*
* int32_t puts(int8_t* s);
*   Inputs: int_8* s = pointer to a string of characters
*   Return Value: Number of bytes written
*	Function: Output a string to the console
*/

int32_t
puts(int8_t* s)
{
	register int32_t index = 0;
	while(s[index] != '\0') {
		putc(s[index]);
		index++;
	}

	return index;
}

/*
* void putc(uint8_t c);
*   Inputs: uint_8* c = character to print
*   Return Value: void
*	Function: Output a character to the console
*/

void
putc(uint8_t c)
{
    if(c == '\n' || c == '\r') {
        screen_y[get_screen_xy_idx()]++;
        screen_x[get_screen_xy_idx()]=0;

        /* If c is a newline or carriage return, scroll the screen */
        scroll();
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS*screen_y[get_screen_xy_idx()] + screen_x[get_screen_xy_idx()]) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS*screen_y[get_screen_xy_idx()] + screen_x[get_screen_xy_idx()]) << 1) + 1) = cur_attribute;
        screen_x[get_screen_xy_idx()]++;

        /* Check if x is at the end of the line, if yes, go to the next row.
         * This allows for text wrapping.
         */
        if (screen_x[get_screen_xy_idx()] == NUM_COLS) {
            screen_y[get_screen_xy_idx()]++;
            scroll();
        }

        screen_x[get_screen_xy_idx()] %= NUM_COLS;
        screen_y[get_screen_xy_idx()] = (screen_y[get_screen_xy_idx()] + (screen_x[get_screen_xy_idx()] / NUM_COLS)) % NUM_ROWS;
        set_cursor(0);
    }
}

/*
* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
*   Inputs: uint32_t value = number to convert
*			int8_t* buf = allocated buffer to place string in
*			int32_t radix = base system. hex, oct, dec, etc.
*   Return Value: number of bytes written
*	Function: Convert a number to its ASCII representation, with base "radix"
*/

int8_t*
itoa(uint32_t value, int8_t* buf, int32_t radix)
{
	static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int8_t *newbuf = buf;
	int32_t i;
	uint32_t newval = value;

	/* Special case for zero */
	if(value == 0) {
		buf[0]='0';
		buf[1]='\0';
		return buf;
	}

	/* Go through the number one place value at a time, and add the
	 * correct digit to "newbuf".  We actually add characters to the
	 * ASCII string from lowest place value to highest, which is the
	 * opposite of how the number should be printed.  We'll reverse the
	 * characters later. */
	while(newval > 0) {
		i = newval % radix;
		*newbuf = lookup[i];
		newbuf++;
		newval /= radix;
	}

	/* Add a terminating NULL */
	*newbuf = '\0';

	/* Reverse the string and return */
	return strrev(buf);
}

/*
* int8_t* strrev(int8_t* s);
*   Inputs: int8_t* s = string to reverse
*   Return Value: reversed string
*	Function: reverses a string s
*/

int8_t*
strrev(int8_t* s)
{
	register int8_t tmp;
	register int32_t beg=0;
	register int32_t end=strlen(s) - 1;

	while(beg < end) {
		tmp = s[end];
		s[end] = s[beg];
		s[beg] = tmp;
		beg++;
		end--;
	}

	return s;
}

/*
* uint32_t strlen(const int8_t* s);
*   Inputs: const int8_t* s = string to take length of
*   Return Value: length of string s
*	Function: return length of string s
*/

uint32_t
strlen(const int8_t* s)
{
	register uint32_t len = 0;
	while(s[len] != '\0')
		len++;

	return len;
}

/*
* void* memset(void* s, int32_t c, uint32_t n);
*   Inputs: void* s = pointer to memory
*			int32_t c = value to set memory to
*			uint32_t n = number of bytes to set
*   Return Value: new string
*	Function: set n consecutive bytes of pointer s to value c
*/

void*
memset(void* s, int32_t c, uint32_t n)
{
	c &= 0xFF;
	asm volatile("                  \n\
			.memset_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memset_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memset_aligned \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memset_top     \n\
			.memset_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     stosl           \n\
			.memset_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memset_done    \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%edx       \n\
			jmp     .memset_bottom  \n\
			.memset_done:           \n\
			"
			:
			: "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memset_word(void* s, int32_t c, uint32_t n);
*   Inputs: void* s = pointer to memory
*			int32_t c = value to set memory to
*			uint32_t n = number of bytes to set
*   Return Value: new string
*	Function: set lower 16 bits of n consecutive memory locations of pointer s to value c
*/

/* Optimized memset_word */
void*
memset_word(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosw           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memset_dword(void* s, int32_t c, uint32_t n);
*   Inputs: void* s = pointer to memory
*			int32_t c = value to set memory to
*			uint32_t n = number of bytes to set
*   Return Value: new string
*	Function: set n consecutive memory locations of pointer s to value c
*/

void*
memset_dword(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosl           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memcpy(void* dest, const void* src, uint32_t n);
*   Inputs: void* dest = destination of copy
*			const void* src = source of copy
*			uint32_t n = number of byets to copy
*   Return Value: pointer to dest
*	Function: copy n bytes of src to dest
*/

void*
memcpy(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			.memcpy_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memcpy_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memcpy_aligned \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memcpy_top     \n\
			.memcpy_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     movsl           \n\
			.memcpy_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memcpy_done    \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%edx       \n\
			jmp     .memcpy_bottom  \n\
			.memcpy_done:           \n\
			"
			:
			: "S"(src), "D"(dest), "c"(n)
			: "eax", "edx", "memory", "cc"
			);

	return dest;
}

/*
* void* memmove(void* dest, const void* src, uint32_t n);
*   Inputs: void* dest = destination of move
*			const void* src = source of move
*			uint32_t n = number of byets to move
*   Return Value: pointer to dest
*	Function: move n bytes of src to dest
*/

/* Optimized memmove (used for overlapping memory areas) */
void*
memmove(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			cmp     %%edi, %%esi    \n\
			jae     .memmove_go     \n\
			leal    -1(%%esi, %%ecx), %%esi    \n\
			leal    -1(%%edi, %%ecx), %%edi    \n\
			std                     \n\
			.memmove_go:            \n\
			rep     movsb           \n\
			"
			:
			: "D"(dest), "S"(src), "c"(n)
			: "edx", "memory", "cc"
			);

	return dest;
}

/*
* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
*   Inputs: const int8_t* s1 = first string to compare
*			const int8_t* s2 = second string to compare
*			uint32_t n = number of bytes to compare
*	Return Value: A zero value indicates that the characters compared
*					in both strings form the same string.
*				A value greater than zero indicates that the first
*					character that does not match has a greater value
*					in str1 than in str2; And a value less than zero
*					indicates the opposite.
*	Function: compares string 1 and string 2 for equality
*/

int32_t
strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
{
	int32_t i;
	for(i=0; i<n; i++) {
		if( (s1[i] != s2[i]) ||
				(s1[i] == '\0') /* || s2[i] == '\0' */ ) {

			/* The s2[i] == '\0' is unnecessary because of the short-circuit
			 * semantics of 'if' expressions in C.  If the first expression
			 * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
			 * s2[i], then we only need to test either s1[i] or s2[i] for
			 * '\0', since we know they are equal. */

			return s1[i] - s2[i];
		}
	}
	return 0;
}

/*
* int8_t* strcpy(int8_t* dest, const int8_t* src)
*   Inputs: int8_t* dest = destination string of copy
*			const int8_t* src = source string of copy
*   Return Value: pointer to dest
*	Function: copy the source string into the destination string
*/

int8_t*
strcpy(int8_t* dest, const int8_t* src)
{
	int32_t i=0;
	while(src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}

	dest[i] = '\0';
	return dest;
}

/*
* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
*   Inputs: int8_t* dest = destination string of copy
*			const int8_t* src = source string of copy
*			uint32_t n = number of bytes to copy
*   Return Value: pointer to dest
*	Function: copy n bytes of the source string into the destination string
*/

int8_t*
strncpy(int8_t* dest, const int8_t* src, uint32_t n)
{
	int32_t i=0;
	while(src[i] != '\0' && i < n) {
		dest[i] = src[i];
		i++;
	}

	while(i < n) {
		dest[i] = '\0';
		i++;
	}

	return dest;
}

/*
* void test_interrupts(void)
*   Inputs: void
*   Return Value: void
*	Function: increments video memory. To be used to test rtc
*/

void
test_interrupts(void)
{
	int32_t i;
	for (i=0; i < NUM_ROWS*NUM_COLS; i++) {
		video_mem[i<<1]++;
	}
}


/* Custom functions written by group OScelot */

/*
 * scroll
 *   DESCRIPTION:  Scrolls the screen when the text has reached the end
 *                 of the screen.
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   sIDE EFFECTS: Overwrites the video memory as it shifts the video memory
 *                 data from the bottom line to the line above.
 */
void scroll(void) {
    int32_t i;

    /* If the y position of the text is in the last row, shift the data up */
	if (screen_y[get_screen_xy_idx()] >= NUM_ROWS) {
		memmove((uint8_t *)video_mem, (uint8_t *)(video_mem + 2 * NUM_COLS),
			     2 * (NUM_ROWS - 1) * NUM_COLS);

        screen_y[get_screen_xy_idx()]--;

        /* Similar to clear(), but instead of clearing the whole video memory,
         * this will only clear the last row of video memory.
         */
        for (i = (NUM_ROWS - 1) * NUM_COLS; i < (NUM_ROWS * NUM_COLS); i++) {
            *(uint8_t *)(video_mem + (i << 1)) = ' ';
            *(uint8_t *)(video_mem + (i << 1) + 1) = cur_attribute;
        }
	}
}

/*
 * set_pos
 *   DESCRIPTION:  Updates the member variables pos_x and pos_y
 *                 of the pos_t struct.
 *   INPUTS:       (x, y) - New x and y values to update struct with
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites the pos_t struct variables of pos_x and pos_y
 */
void set_pos(int x, int y) {

	/* If we're past the end of the screen, go back
	 * one column and get to the next row.
	 */
    while (x >= NUM_COLS) {
        x -= NUM_COLS;
        y++;
    }

    /* If we're below the bottom of the screen, reduce y */
    while (y >= NUM_ROWS) {
        y--;
    }

    screen_x[get_screen_xy_idx()] = x;
    screen_y[get_screen_xy_idx()] = y;
}

/*
 * get_pos
 *   DESCRIPTION:  Gets the current values of pos_x and pos_y variables in
 *                 the pos_t struct.
 *   INPUTS:       none
 *   OUTPUTS:      none
 *   RETURN VALUE: a struct with screen_x[get_screen_xy_idx()] and screen_y[get_screen_xy_idx()] as the pos_x and pos_y values
 *   SIDE EFFECTS: Creates a new pos_t struct
 */
pos_t get_pos(void) {
    pos_t cur_pos;

    cur_pos.x = screen_x[get_screen_xy_idx()];
    cur_pos.y = screen_y[get_screen_xy_idx()];

    return cur_pos;
}

/*
 * set_cursor
 *   DESCRIPTION:  Changes the cursor by the requested x offset in the terminal.
 *   INPUTS:       x - offset from current cursor position in which to put cursor
 *   OUTPUTS:      none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites video memory to move cursor.
 */
void set_cursor(int x) {
	int new_cursor;
	pos_t cur_cursor;

    if (video_mem != (char*)VIDEO) {
        return;
    }

	/* Get the current cursor position and update the cursor with the offset */
	cur_cursor = get_pos();
	new_cursor = cur_cursor.x + x + (cur_cursor.y * NUM_COLS);

	// if (x == -1) {
	// 	set_pos(cur_cursor.pos_x - 1, cur_cursor.pos_y);
	// } else if (x == 1) {
	// 	set_pos(cur_cursor.pos_x + 1, cur_cursor.pos_y);
	// }

	/* Accessing the appropriate cursor registers in the VGA */
	outb(CURSOR_LOW_REG, CRTC_ADDR_REG);
	outb((uint8_t)new_cursor, CRTC_DATA_REG);

	outb(CURSOR_HIGH_REG, CRTC_ADDR_REG);
	outb((uint8_t)(new_cursor >> 8), CRTC_DATA_REG);
}

/*
* set_attribute
*   DESCRIPTION:  Changes the current attribute being used.
*	INPUTS:		  attribute - attribute to change to
*   OUTPUTS:	  none
*   RETURN VALUE: none
*	SIDE EFFECTS: Overwrites video memory to change color
*/
void set_attribute(char attribute) {
	cur_attribute = attribute;
}
