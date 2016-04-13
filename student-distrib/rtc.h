// rtc.h
// header for rtc driver

#ifndef RTC_H
#define RTC_H

#include "types.h"

extern void rtc_init();
extern void rtcHandler();

extern int32_t rtc_open(const int8_t *filename);
extern int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t rtc_close(int32_t fd);

void rtc_test1();
void rtc_test2();
#endif // RTC_H
