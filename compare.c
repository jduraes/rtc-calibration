#include <stdio.h>
#include "rtc.h"

int main() {
    RTC_Time time;
    int test_result, get_result;
    
    printf("RTC Function Comparison Test\n");
    
    // Test 1: Direct HBIOS test
    test_result = hbios_rtc_test();
    printf("1. hbios_rtc_test result: 0x%02X\n", test_result);
    
    // Test 2: Get time through wrapper
    get_result = hbios_rtc_get_time(&time);
    printf("2. hbios_rtc_get_time result: 0x%02X\n", get_result);
    
    // Test 3: Show the actual time data
    printf("3. Data from get_time: %02X %02X %02X %02X %02X %02X\n",
           time.second, time.minute, time.hour, 
           time.date, time.month, time.year);
    
    printf("Test complete.\n");
    return 0;
}
