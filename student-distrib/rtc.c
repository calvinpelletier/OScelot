// rtc.c
// rtc device driver

#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "filesys.h"

// CONSTANTS
#define RTC_ADDR 0x70 // port for addressing RTC registers and enabling/disabling NMIs
#define RTC_DATA 0x71 // port for writing data to RTC registers
#define TEST_RTC_RTC 1

#if (TEST_RTC_RTC)
volatile int count = 0;
#endif

// FUNCTION DECLARATIONS
void rtc_init(void);
void rtcHandler(void);

// Flags for rtc_read
volatile char rtc_interrupt_flag;
static int rtc_in_use;
int freq;

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
    rtc_in_use = 0;
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

    if (TEST_RTC_RTC) {
        // printf("DEBUG: received RTC interrupt %d.\n", count);
        // count++;
        // test_interrupts();
    }

    send_eoi(RTC_IRQ_NUM);
    rtc_interrupt_flag = 1;
}

/*
 * rtc_open
 * DESCRIPTION: Opens the RTC driver stream
 * INPUTS: filename - the pointer to the filename, not used
 * OUTPUTS: none
 * RETURNS: -1 always, file descriptor not used
 * NOTES:
 */
int32_t rtc_open(const int8_t *filename)
{
    if(rtc_in_use == 1)
        return -1;

    rtc_in_use = 1;

    cli();
    unsigned char rate = 0x0F;
    outb(0x8A, RTC_ADDR);
    unsigned char prev = inb(RTC_DATA);
    unsigned char newRate = (prev & 0xF0) | rate;
    outb(0x8A, RTC_ADDR);
    outb(newRate, RTC_DATA);
    sti();

    return -1;
}

/*
 * rtc_read
 * DESCRIPTION:
 * INPUTS:
 * OUTPUTS:
 * RETURNS:
 * NOTES:
 */
int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes)
{
    if (rtc_in_use != 1)
        return -1;

    rtc_interrupt_flag = 0;
    while(rtc_interrupt_flag != 1) {
        // wait for rtc_interrupt_flag to be set to one.
    }
    count = 0;
    return 0;
}

/*
 * rtc_write
 * DESCRIPTION: Changes RTC interrupt frequency by a specified rate
 * INPUTS: fd     - not used
 *         buf    -
 *         nbytes -
 * OUTPUTS: none
 * RETURNS: The number of bytes written.
 * NOTES:
 */
int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes)
{
    if (rtc_in_use != 1)
        return -1;

    int num_bytes = 0;

    if (buf != NULL) {
        int frequency = *(int*)buf;

        /*
         * If the rate is not in the range of acceptable values,
         * fail gracefully
         */
        if (frequency < 2 || frequency  > 1024)
            return -1;

        /*
         * If the rate is not a power of two,
         * fail gracefully
         */
        int rate = 1;
        while ((1 << rate) < frequency)
            rate++;

        if (frequency != 1 << rate)
            return -1;

        cli();
        /*
         * get the value of the old frequency
         */
        outb(0x8A, RTC_ADDR);
        num_bytes++;
        unsigned char newRate = inb(RTC_DATA);

        /*
         * set the value of the new frequency
         */

        newRate = (newRate & 0xF0) | (16 - rate);

        outb(0x8A, RTC_ADDR);
        num_bytes++;
        outb(newRate, RTC_DATA);
        num_bytes++;
        sti();
    } else {
        return -1;
    }
    return num_bytes;
}

/*
 * rtc_close
 * DESCRIPTION:
 * INPUTS:
 * OUTPUTS:
 * RETURNS:
 * NOTES:
 */
int32_t rtc_close(int32_t fd)
{
    if (rtc_in_use == 0)
        return -1;
    rtc_in_use = 0;
    return 0;
}
