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
    // initialize master and slave
    outb(ICW1, MASTER_CMD); // begin sequence
    outb(ICW1, SLAVE_CMD);
    outb(ICW2_MASTER, MASTER_DATA); // specify port #
    outb(ICW2_SLAVE, SLAVE_DATA);
    outb(ICW3_MASTER, MASTER_DATA); // info about slave (connected to line 4)
    outb(ICW3_SLAVE, SLAVE_DATA); // cascade info
    outb(ICW4, MASTER_DATA); // extra info
    outb(ICW4, SLAVE_DATA); // extra info

    // DEBUG: verify that all interrupts are indeed masked
    if (DEBUG) {
        if (inb(MASTER_DATA) || inb(SLAVE_DATA)) {
            printf("ERROR: master and/or slave interrupts are not all masked after PIC initialization.\n");
        }
    }

    enable_irq(2); // slave is connected to port 2 on master
}


/*
enable_irq
    DESCRIPTION: enables (unmasks) a specific irq
    INPUTS: the number (0-15) of the irq
    OUTPUTS: none
    RETURNS: none
*/
void enable_irq(unsigned int irq_num) {
    // check if irq is on master or slave
    if (irq_num < 8) { // master
        if (master_mask & (1 << irq_num)) { // not already enabled
        	master_mask &= ~(1 << irq_num);
            outb(master_mask, MASTER_DATA);
        }
    } else if (irq_num < 16) { // slave
        irq_num -= 8;
        if (slave_mask & (1 << irq_num)) { // not already enabled
        	slave_mask &= ~(1 << irq_num);
            outb(slave_mask, SLAVE_DATA);
        }
    } else { // error
        printf("ERROR: invalid irq_num in enable_irq.\n");
    }
}


/*
disable_irq
    DESCRIPTION: disables (masks) a specific irq
    INPUTS: the number (0-15) of the irq
    OUTPUTS: none
    RETURNS: none
*/
void disable_irq(unsigned int irq_num) {
    // check if irq is on master or slave
    if (irq_num < 8) { // master
        if (!(master_mask & (1 << irq_num))) { // not already disabled
        	master_mask |= (1 << irq_num);
            outb(master_mask, MASTER_DATA);
        }
    } else if (irq_num < 16) { // slave
        irq_num -= 8;
        if (!(slave_mask & (1 << irq_num))) { // not already disabled
        	slave_mask |= (1 << irq_num);
            outb(slave_mask, SLAVE_DATA);
        }
    } else { // error
        printf("ERROR: invalid irq_num in disable_irq.\n");
    }
}


/*
send_eoi
    DESCRIPTION: sends EOI to PIC letting it know that a specific IRQ is done being serviced.
    INPUTS: the number (0-15) of the irq
    OUTPUTS: none
    RETURNS: none
*/
void send_eoi(unsigned int irq_num) {
    if (irq_num > 15) {return;}
    //enable_irq(irq_num); // unmask irq
    // NOTE: not sure if I'm using EOI correctly... if it doesn't work try sending it over data line?
    if (irq_num < 8) { // master
        outb(EOI | (unsigned char)(irq_num), MASTER_CMD);
    } else { // slave
    	//enable_irq(2); // unmask master IRQ2, which slave PIC is connected to
        outb(EOI | (unsigned char)(irq_num - 8), SLAVE_CMD);
        outb(EOI | (unsigned char)(2), MASTER_CMD); // let master know as well

        // verify that no interrupts are being serviced
        if (DEBUG) {
            outb(0x0B, MASTER_CMD); // 0x0B tells the PIC to ready ISR for a read
            outb(0x0B, SLAVE_CMD);
            if (inb(MASTER_CMD) || inb(SLAVE_CMD)) {
                printf("WARNING: ISR is not 0 after sending EOI. Either the EOI function is not working, or another interrupt was serviced immediately.\n");
            }
        }
    }
}
