// rtc.h
// header for rtc driver

#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "filesys.h"

extern void rtc_init();
extern void rtcHandler();

extern int32_t rtc_open();
extern int32_t rtc_read(file_t * file, uint8_t * buf, int32_t nbytes);
extern int32_t rtc_write(file_t * file, uint8_t * buf, int32_t nbytes);
extern int32_t rtc_close(file_t * file);

void rtc_test1();
void rtc_test2();
#endif // RTC_H
