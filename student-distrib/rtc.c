// rtc.c
// rtc device driver

#include "rtc.h"
#include "lib.h"
#include "i8259.h"


// CONSTANTS
#define RTC_ADDR 0x70 // port for addressing RTC registers and enabling/disabling NMIs
#define RTC_DATA 0x71 // port for writing data to RTC registers

#if (DEBUG_ALL)
static int count = 0;
#endif


// FUNCTION DECLARATIONS
void rtc_init(void);
void rtcHandler(void);


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
    enable_irq(RTC_IRQ_NUM);
}


/*
rtcHandler
    DESCRIPTION: called on RTC interrupts
    INPUTS: none
    OUTPUTS: none
    RETURNS: none
    NOTES: important that interrupts are disabled when calling this function
*/
void rtcHandler(void) {
    outb(0x0C, RTC_ADDR); // select register 0x0C
    inb(RTC_DATA); // throw away contents (important)

    if (DEBUG_ALL) {
        // printf("DEBUG: received RTC interrupt %d.\n", count);
        // count++;
        test_interrupts();
    }

    send_eoi(RTC_IRQ_NUM);
}

/*
 * open
 * DESCRIPTION:
 * INPUTS:
 * OUTPUTS:
 * RETURNS:
 * NOTES:
 */
int32_t void open(const uint8_t *filename)
{

}

/*
 * read
 * DESCRIPTION:
 * INPUTS:
 * OUTPUTS:
 * RETURNS:
 * NOTES:
 */
int32_t void read(int32_t fd, void *buf, int32_t nbytes)
{

}

/*
 * write
 * DESCRIPTION: Changes RTC interrupt frequency by a specified rate
 * INPUTS:
 * OUTPUTS:
 * RETURNS:
 * NOTES:
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    char rate = (char) buf;
    rate &= 0x0F;
    outb(0x70, 0x8A);
    char prev=inb(0x7a);
    outb(0x70, 0x8A);
    outb(0x71, (prev & 0xF0) | rate);
    printf("rate changeed");
    return 0;
}

/*
 * close
 * DESCRIPTION:
 * INPUTS:
 * OUTPUTS:
 * RETURNS:
 * NOTES:
 */
int32_t void close(int32_t fd)
{

}
