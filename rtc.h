#ifndef RTC_H
#define RTC_H

typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char date;
    unsigned char month;
    unsigned char year;
} RTC_Time;

// Function prototypes for HBIOS RTC access
int hbios_rtc_detect(void);
int hbios_rtc_get_time(RTC_Time *time);
int hbios_rtc_set_time(const RTC_Time *time);
int hbios_rtc_test(void);

#endif // RTC_H
