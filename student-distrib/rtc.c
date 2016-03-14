// rtc.c
// rtc device driver

#include "rtc.h"
#include "lib.h"
#include "i8259.h"


// CONSTANTS
#define DEBUG 1
#define RTC_ADDR 0x70 // port for addressing RTC registers and enabling/disabling NMIs
#define RTC_DATA 0x71 // port for writing data to RTC registers


// FUNCTION DECLARATIONS
void rtc_init(void);
void rtc_handler(void);


// GLOBAL FUNCTIONS
/*
rtc_init
    DESCRIPTION: initializes the rtc chip
    INPUTS: none
    OUTPUTS: none
    RETURNS: none
    NOTES: important that interrupts are disabled when calling this function
*/
void rtc_init(void) {
    outb(0x8B, RTC_ADDR); // address register 0x0B and disable NMIs (0x80)
    unsigned char temp = inb(RTC_DATA); // read register 0x0B
    outb(0x8B, RTC_ADDR); // address register again because apparently reading resets this
    outb(temp | 0x40, RTC_DATA); // turns on periodic interrupts
    // outb(0x0B, RTC_ADDR); // reenable NMIs and I guess just address 0x0B just for the hell of it
}


/*
rtc_handler
    DESCRIPTION: called on RTC interrupts
    INPUTS: none
    OUTPUTS: none
    RETURNS: none
    NOTES: important that interrupts are disabled when calling this function
*/
void rtc_handler(void) {
    outb(0x0C, RTC_ADDR); // select register 0x0C
    inb(RTC_DATA); // throw away contents (important)
    if (DEBUG) {
        printf("DEBUG: received RTC interrupt.\n");
        test_interrupts();
    }
    send_eoi(8);
}


// NOTE: it's important to read from register 0x0C (type of interrupt) on every interrupt
//       even if we don't care about it, otherwise interrupts will stop being generated.
