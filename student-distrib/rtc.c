// rtc.c
// rtc device driver

#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "filesys.h"
#include "syscalls.h"

// CONSTANTS
#define RTC_ADDR 0x70 // port for addressing RTC registers and enabling/disabling NMIs
#define RTC_DATA 0x71 // port for writing data to RTC registers
#define MAXIMUM_RTC_RATE 1024
#define NUM_FREQS 10

// FUNCTION DECLARATIONS
void rtc_init();
void rtcHandler();

// Flags for rtc_read
int active_freq[MAX_PROCESSES + 1];
volatile int8_t interrupt_flag[MAX_PROCESSES + 1];
int count = 0;

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
    int i;
    for (i = 0; i < MAX_PROCESSES + 1; i++) {
        active_freq[i] = 0;
        interrupt_flag[i] = 0;
    }

    active_freq[0] = 32;

    outb(0x8B, RTC_ADDR); // address register 0x0B and disable NMIs (0x80)
    uint8_t  temp = inb(RTC_DATA); // read register 0x0B
    outb(0x8B, RTC_ADDR); // address register again because apparently reading resets this
    outb(temp | 0x40, RTC_DATA); // turns on periodic interrupts at 1024hz
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
    cli();
    disable_irq(RTC_IRQ_NUM);

    outb(0x0C, RTC_ADDR); // select register 0x0C
    inb(RTC_DATA); // throw away contents (important)

    int old_count = count;
    count++;
    int mask = 0x00000001;
    int i, j;
    for (i = 0; i < NUM_FREQS; i++) {
        if ((old_count & mask) != (count & mask)) {
            for (j = 0; j < MAX_PROCESSES + 1; j++) {
                if (active_freq[j] == (MAXIMUM_RTC_RATE >> i)) {
                    interrupt_flag[j] = 1;
                }
            }
        }
        mask = mask << 1;
    }

    if ((CPID != 0) && interrupt_flag[0]) {
        interrupt_flag[0] = 0;
        send_eoi(RTC_IRQ_NUM);
        enable_irq(RTC_IRQ_NUM);
        task_switch();
    }

    send_eoi(RTC_IRQ_NUM);
    enable_irq(RTC_IRQ_NUM);
    sti();
}

/*
 * rtc_open
 * DESCRIPTION: Opens the RTC driver stream
 * INPUTS: none
 * OUTPUTS: none
 * RETURNS: 0
 * NOTES:
 */
int32_t rtc_open()
{
    return 0;
}

/*
 * rtc_read
 * DESCRIPTION: Reads from the RTC driver
 * INPUTS: file    - not used
 *         buf    - not used
 *         nbytes - not used
 * OUTPUTS: none
 * RETURNS: The number of bytes written, -1 if read not successful.
 * NOTES:
 */
int32_t rtc_read(file_t * file, uint8_t *buf, int32_t nbytes)
{
    while(!interrupt_flag[CPID]) {
        sti();
    }

    interrupt_flag[CPID] = 0;

    return nbytes;
}

/*
 * rtc_write
 * DESCRIPTION: Sets the RTC interrupt frequency to a specified rate
 * INPUTS: file    - not used
 *         buf    - The new rate to set the RTC Periodic Interrupt to
 *         nbytes - The number of bytes to write, not used.
 * OUTPUTS: none
 * RETURNS: The number of bytes written. -1 if it fails.
 * NOTES:
 */
int32_t rtc_write(file_t * file, uint8_t *buf, int32_t nbytes)
{
    if (buf == 0) {
        return -1;
    }

    active_freq[CPID] = *((int*)buf);
    return nbytes;
}

/*
 * rtc_close
 * DESCRIPTION: Closes the RTC driver stream thing
 * INPUTS: file - not used
 * OUTPUTS: non
 * RETURNS: 0 if the driver is closed properly, -1 otherwise.
 * NOTES:
 */
int32_t rtc_close(file_t* file)
{
    return 0;
}
