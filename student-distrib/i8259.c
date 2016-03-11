/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


// CONSTANTS
#define DEBUG 1


// FUNCTION DECLARATIONS
void i8259_init(void);
void enable_irq(unsigned int irq_num);
void disable_irq(unsigned int irq_num);
void send_eoi(unsigned int irq_num);


// GLOBAL VARIABLES
/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
unsigned char master_mask; /* IRQs 7-0 */
unsigned char slave_mask; /* IRQs 15-8 */


/* Initialize the 8259 PIC */
void i8259_init(void) {
    // mask all interrupts
    outb(MASTER_DATA, 0xFF);
    outb(SLAVE_DATA, 0xFF);
    master_mask = 0xFF;
    slave_mask = 0xFF;

    // TODO: should we wait at all between commands?
    // initialize master
    outb(MASTER_CMD, ICW1); // begin sequence
    outb(MASTER_DATA, ICW2_MASTER); // specify port #
    outb(MASTER_DATA, ICW3_MASTER); // info about slave
    outb(MASTER_DATA, ICW4); // extra info

    // initialize slave
    outb(SLAVE_CMD, ICW1);
    outb(SLAVE_DATA, ICW2_SLAVE);
    outb(SLAVE_DATA, ICW3_SLAVE); // cascade info
    outb(SLAVE_DATA, ICW4) // extra info

    // DEBUG: verify that all interrupts are indeed masked
    if (DEBUG) {
        if (inb(MASTER_DATA) || inb(SLAVE_DATA)) {
            printf("ERROR: master and/or slave interrupts are not all masked after PIC initialization.")
        }
    }

    //TODO: initialize devices
}


/* Enable (unmask) the specified IRQ */
void enable_irq(unsigned int irq_num) {
    // check if irq is on master or slave
    if (irq_num < 8) { // master
        if (master_mask & (1 << irq_num)) { // not already enabled
            outb(MASTER_DATA, inb(MASTER_DATA) & ~(1 << irq_num));
            master_mask &= ~(1 << irq_num);
        }
    } else if (irq_num < 16) { // slave
        irq_num -= 8;
        if (slave_mask & (1 << irq_num)) { // not already enabled
            outb(SLAVE_DATA, inb(SLAVE_DATA) & ~(1 << irq_num));
            slave_mask &= ~(1 << irq_num);
        }
    } else { // error
        printf("ERROR: invalid irq_num in enable_irq.");
    }
}


/* Disable (mask) the specified IRQ */
void disable_irq(unsigned int irq_num) {
    // check if irq is on master or slave
    if (irq_num < 8) { // master
        if (!(master_mask & (1 << irq_num))) { // not already disabled
            outb(MASTER_DATA, inb(MASTER_DATA) | (1 << irq_num));
            master_mask |= ~(1 << irq_num);
        }
    } else if (irq_num < 16) { // slave
        irq_num -= 8;
        if (!(slave_mask & (1 << irq_num))) { // not already disabled
            outb(SLAVE_DATA, inb(SLAVE_DATA) | (1 << irq_num));
            slave_mask |= (1 << irq_num);
        }
    } else { // error
        printf("ERROR: invalid irq_num in disable_irq.");
    }
}


/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(unsigned int irq_num) {
}
