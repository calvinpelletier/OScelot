// rtc.h
// header for rtc driver

#ifndef RTC_H
#define RTC_H

#include "types.h"

extern void rtc_init(void);
extern void rtcHandler(void);

extern void open();
extern void read();
extern void write();
extern void close();

#endif // RTC_H
