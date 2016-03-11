// rtc.c
// rtc device driver

#include "rtc.h"
#include "lib.h"
#include "i8259.h"


// CONSTANTS
#define RTC_ADDR 0x70 // port for addressing RTC registers and enabling/disabling NMIs
#define RTC_DATA 0x71 // port for writing data to RTC registers
#define RTC_IRQ 0x08


// FUNCTION DECLARATIONS
void rtc_init(void);


// GLOBAL FUNCTIONS
void rtc_init(void) {
    // WARNING: important that interrupts are disabled when calling this funtion
    outb(RTC_ADDR, 0x8B); // address register 0x0B and disable NMIs (0x80)
    unsigned char temp = inb(RTC_DATA); // read register 0x0B
    outb(RTC_ADDR, 0x8B); // address register again because apparently reading resets this
    outb(RTC_DATA, 0x40 | temp); // turns on periodic interrupts
    enable_irq(RTC_IRQ);
    // NOTE: should we enable NMIs?
}

// NOTE: it's important to read from register 0x0C (type of interrupt) on every interrupt
//       even if we don't care about it, otherwise interrupts will stop being generated.
