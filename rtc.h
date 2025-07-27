#ifndef RTC_H
#define RTC_H

#include <stdint.h>

// RTC time structure
typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} RTC_Time;

// Function prototypes for HBIOS RTC access
int hbios_rtc_detect(void);
int hbios_rtc_get_time(RTC_Time *time);
int hbios_rtc_set_time(const RTC_Time *time);
int hbios_rtc_test(void);

#endif // RTC_H
