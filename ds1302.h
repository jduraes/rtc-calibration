#ifndef DS1302_H
#define DS1302_H

#include <stdint.h>

// Time structure for DS1302
typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} DS1302_Time;

// Initialize the DS1302 chip
void ds1302_init();

// Read time from DS1302
void ds1302_get_time(DS1302_Time *time);

// Set time to DS1302
void ds1302_set_time(const DS1302_Time *time);

// Detect the DS1302 chip
uint8_t ds1302_detect();

#endif // DS1302_H

