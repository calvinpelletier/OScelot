/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

// useful IRQ numbers
#define RTC_IRQ_NUM      8
#define KEYBOARD_IRQ_NUM 1
#define PIT_IRQ_NUM      0
#define SLAVE_IRQ_NUM    2

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init();
/* Enable (unmask) the specified IRQ */
void enable_irq(uint8_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint8_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint8_t irq_num);

#endif /* _I8259_H */
