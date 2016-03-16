/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


// CONSTANTS
/* Ports that each PIC sits on */
#define MASTER_CMD    0x20
#define MASTER_DATA   0x21
#define SLAVE_CMD    0xA0
#define SLAVE_DATA   0xA1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1    0x11
#define ICW2_MASTER   0x20
#define ICW2_SLAVE    0x28
#define ICW3_MASTER   0x04
#define ICW3_SLAVE    0x02
#define ICW4          0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI             0x60


// FUNCTION DECLARATIONS
void i8259_init(void);
void enable_irq(unsigned char irq_num);
void disable_irq(unsigned char irq_num);
void send_eoi(unsigned char irq_num);


// GLOBAL VARIABLES
/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
static unsigned char master_mask; /* IRQs 7-0 */
static unsigned char slave_mask; /* IRQs 15-8 */


/*
i8259_init
    DESCRIPTION: initializes the PIC
    INPUTS: none
    OUTPUTS: none
    RETURNS: none
*/
void i8259_init(void) {
    // mask all interrupts
    outb(0xFF, MASTER_DATA);
    outb(0xFF, SLAVE_DATA);
    master_mask = 0xFF;
    slave_mask = 0xFF;

    // NOTE: should we wait at all between commands?
    // initialize master
    outb(ICW1, MASTER_CMD); // begin sequence
    outb(ICW2_MASTER, MASTER_DATA); // specify port #
    outb(ICW3_MASTER, MASTER_DATA); // info about slave (connected to line 4)
    outb(ICW4, MASTER_DATA); // extra info

    // initialize slave
    outb(ICW1, SLAVE_CMD);
    outb(ICW2_SLAVE, SLAVE_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA); // cascade info
    outb(ICW4, SLAVE_DATA); // extra info

    // DEBUG: verify that all interrupts are indeed masked
    if (DEBUG_ALL) {
        if (inb(MASTER_DATA) != 0xFF || inb(SLAVE_DATA) != 0xFF) {
            printf("ERROR: master and/or slave interrupts are not all masked after PIC initialization.\n");
        }
    }

    enable_irq(2); // unmask slave line
}


/*
enable_irq
    DESCRIPTION: enables (unmasks) a specific irq
    INPUTS: the number (0-15) of the irq
    OUTPUTS: none
    RETURNS: none
*/
void enable_irq(unsigned char irq_num) {
    // check if irq is on master or slave
    if (irq_num < 8) { // master
        if (master_mask & (0x01 << irq_num)) { // not already enabled
        	master_mask &= ~(0x01 << irq_num);
            outb(master_mask, MASTER_DATA);
        }
    } else { // slave
        irq_num -= 8;
        if (slave_mask & (0x01 << irq_num)) { // not already enabled
        	slave_mask &= ~(0x01 << irq_num);
            outb(slave_mask, SLAVE_DATA);
        }
    }
}


/*
disable_irq
    DESCRIPTION: disables (masks) a specific irq
    INPUTS: the number (0-15) of the irq
    OUTPUTS: none
    RETURNS: none
*/
void disable_irq(unsigned char irq_num) {
    // check if irq is on master or slave
    if (irq_num < 8) { // master
        if (!(master_mask & (0x01 << irq_num))) { // not already disabled
        	master_mask |= 0x01 << irq_num;
            outb(master_mask, MASTER_DATA);
        }
    } else { // slave
        irq_num -= 8;
        if (!(slave_mask & (0x01 << irq_num))) { // not already disabled
        	slave_mask |= 0x01 << irq_num;
            outb(slave_mask, SLAVE_DATA);
        }
    }
}


/*
send_eoi
    DESCRIPTION: sends EOI to PIC letting it know that a specific IRQ is done being serviced.
    INPUTS: the number (0-15) of the irq
    OUTPUTS: none
    RETURNS: none
*/
void send_eoi(unsigned char irq_num) {
    if (irq_num < 8) { // master
        outb(EOI | irq_num, MASTER_CMD);
    } else { // slave
        outb(EOI | (irq_num - 8), SLAVE_CMD);
        outb(EOI | 2, MASTER_CMD); // let master know as well
    }
}
