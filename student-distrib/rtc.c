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
#define MAXIMUM_RTC_RATE 1024

#if (TEST_RTC_RTC)
volatile int32_t  count = 0;
#endif

// FUNCTION DECLARATIONS
void rtc_init();
void rtcHandler();

// Flags for rtc_read
volatile int8_t  rtc_interrupt_flag;
static int32_t  rtc_in_use;
int32_t  freq;

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
    uint8_t  temp = inb(RTC_DATA); // read register 0x0B
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
 * RETURNS: 0 on successful open, -1 if failed to open
 * NOTES:
 */
int32_t rtc_open(const int8_t *filename)
{
    if(rtc_in_use == 1)
        return -1;

    rtc_in_use = 1;

    cli();
    uint8_t  rate = 0x0F;
    outb(0x8A, RTC_ADDR);
    uint8_t  prev = inb(RTC_DATA);
    uint8_t  newRate = (prev & 0xF0) | rate;
    outb(0x8A, RTC_ADDR);
    outb(newRate, RTC_DATA);
    sti();

    return 0;
}

/*
 * rtc_read
 * DESCRIPTION: Reads from the RTC driver
 * INPUTS: fd     - not used
 *         buf    - not used
 *         nbytes - not used
 * OUTPUTS: none
 * RETURNS: The number of bytes written, -1 if read not successful.
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
    // count = 0;
    return 0;
}

/*
 * rtc_write
 * DESCRIPTION: Sets the RTC interrupt frequency to a specified rate
 * INPUTS: fd     - not used
 *         buf    - The new rate to set the RTC Periodic Interrupt to
 *         nbytes - The number of bytes to write, not used.
 * OUTPUTS: none
 * RETURNS: The number of bytes written. -1 if it fails.
 * NOTES:
 */
int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes)
{
    if (rtc_in_use != 1)
        return -1;

    int32_t  num_bytes = 0;

    if (buf != NULL) {
        int32_t  frequency = *(int*)buf;

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
        int32_t  rate = 1;
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
        uint8_t  newRate = inb(RTC_DATA);

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
 * DESCRIPTION: Closes the RTC driver stream thing
 * INPUTS: fd - not used
 * OUTPUTS: non
 * RETURNS: 0 if the driver is closed properly, -1 otherwise.
 * NOTES:
 */
int32_t rtc_close(int32_t fd)
{
    if (rtc_in_use == 0)
        return -1;

    rtc_in_use = 0;
    return 0;
}

void rtc_test1() {
    int32_t  fd = rtc_open("rtc.c");
    if (fd == 0)
        printf("successful open");
    else
        printf("failed to open");

    fd = rtc_close(fd);
    if (fd == 0)
        printf("successful close");
    else
        printf("failed to close");
}

void rtc_test2() {
    int32_t  count = 0;
    int32_t  tmp_fd = rtc_open("rtc.c");
    int32_t  rate = 2;
    int32_t  tmp;
    while (rate <= MAXIMUM_RTC_RATE) {
        tmp = rate;
        int32_t  *newRate = &tmp;
        tmp = rtc_write(tmp_fd, newRate, 0);
        while (count < (rate * 4)) {
            tmp = rtc_read(tmp_fd, newRate, 0);
            printf("current rate is %d interrupts per second: count is %d\n", rate, count);
            count++;
        }
        rate = rate << 1;
        count = 0;
    }
    tmp = rtc_close(tmp_fd);
    if (tmp == 0)
        printf("Closed correctly \n");
    else
        printf("ERROR");
}
