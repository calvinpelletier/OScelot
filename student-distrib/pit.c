// pit.c

#include "syscalls.h"
#include "lib.h"
#include "i8259.h"

// CONSTANTS
#define DATA_PORT 0x40
#define COMMAND_PORT 0x43
#define COUNT 25000
#define MODE 0x30 // channel 0, lo/hi access, mode 0, 16-bit binary

// LOCAL FUNCTION DECLARATIONS
void set_count(int count);

// GLOBAL FUNCTIONS
void pit_init(void) {
    set_count(COUNT);
    enable_irq(PIT_IRQ_NUM);
}

void pitHandler(void) {
    cli();
    disable_irq(PIT_IRQ_NUM);

    set_count(COUNT);

    send_eoi(PIT_IRQ_NUM);
    enable_irq(PIT_IRQ_NUM);

    // make sure the first process has been launched
    if (CPID != 0) {
        task_switch();
    }

    sti();
}

// LOCAL FUNCTIONS
void set_count(int count) {
    outb(MODE, COMMAND_PORT);

    // low 8 bits
    unsigned char temp = (unsigned char)(count & 0x000000FF);
    outb(temp, DATA_PORT);

    // high 8 bits
    temp = (unsigned char)((count & 0x0000FF00) >> 8);
    outb(temp, DATA_PORT);
}
