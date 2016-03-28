// rtc.h
// header for rtc driver

#ifndef RTC_H
#define RTC_H

#include "types.h"

extern void rtc_init(void);
extern void rtcHandler(void);

extern int32_t open(const uint8_t *filename);
extern int32_t read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t close(int32_t fd);

#endif // RTC_H
