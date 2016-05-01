// pit.c

#include "syscalls.h"
#include "lib.h"
#include "i8259.h"

// CONSTANTS
#define DATA_PORT 0x40
#define COMMAND_PORT 0x43
#define COUNT 25000

// LOCAL FUNCTION DECLARATIONS
void set_count(int count);

// GLOBAL FUNCTIONS
void pit_init(void) {
    set_count(COUNT);
    enable_irq(PIT_IRQ_NUM);
}

void pit_handler(void) {
    cli();
    disable_irq(PIT_IRQ_NUM);
    
    send_eoi(RTC_IRQ_NUM);
    enable_irq(RTC_IRQ_NUM);
    sti();
}

// LOCAL FUNCTIONS
void set_count(int count) {

}
