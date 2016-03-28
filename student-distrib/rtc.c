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

// Flag for rtc_read
volatile char rtc_interrupt_flag;

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
    rtc_interrupt_flag = 1;
}

/*
 * rtc_open
 * DESCRIPTION:
 * INPUTS:
 * OUTPUTS:
 * RETURNS:
 * NOTES:
 */
int32_t rtc_open(const uint8_t *filename)
{
    int open_hertz = 2;
    int *pass;
    pass = &open_hertz;

    rtc_write(0, pass, 4);
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
    while(rtc_interrupt_flag != 1) {
        // wait for rtc_interrupt_flag to be set to one.
    }
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
    int i;
    int num_bytes = 0;

    if (buf != NULL) {
        int rate = *(int*)buf;
        int check = *(int*)buf;
        int flag = 1;

        while (check != 1) {
            if (check%2 != 0)
                flag = 0;
            check /= 2;
        }

        /*
         * If the rate is not a power of two
         * or the rate is greater than 1024,
         * fail gracefully
         */
        if (!flag || rate > 1024)
            return -1;

        cli();
        outb(RTC_ADDR, 0x8A);
        num_bytes++;
        char prev = inb(RTC_DATA);
        outb(RTC_ADDR, 0x8A);
        num_bytes++;
        outb(RTC_DATA, rate);
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
    return -1;
}
