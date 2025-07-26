#include "cpm.h"
#include "rp5c01.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Calibration constants
#define CALIB_DURATION_MINUTES 10
#define EXPECTED_TICKS_PER_MINUTE 60
#define RTC_ACCURACY_THRESHOLD 5  // seconds tolerance

// Convert BCD to decimal
inline uint8_t bcdToDecimal(uint8_t bcd) { 
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F); 
}

// Convert decimal to BCD
inline uint8_t decimalToBcd(uint8_t decimal) {
    return ((decimal / 10) << 4) + (decimal % 10);
}

// Convert datetime from BCD to decimal
static void convertFromBcd(rtcDateTime *datetime) __z88dk_fastcall {
    datetime->second = bcdToDecimal(datetime->second);
    datetime->minute = bcdToDecimal(datetime->minute);
    datetime->hour   = bcdToDecimal(datetime->hour);
    datetime->day    = bcdToDecimal(datetime->day);
    datetime->month  = bcdToDecimal(datetime->month);
    datetime->year   = bcdToDecimal(datetime->year);
}

// Convert datetime from decimal to BCD
static void convertToBcd(rtcDateTime *datetime) __z88dk_fastcall {
    datetime->second = decimalToBcd(datetime->second);
    datetime->minute = decimalToBcd(datetime->minute);
    datetime->hour   = decimalToBcd(datetime->hour);
    datetime->day    = decimalToBcd(datetime->day);
    datetime->month  = decimalToBcd(datetime->month);
    datetime->year   = decimalToBcd(datetime->year);
}

rtcDateTime datetime;
char line[10];

// Convert 2-character string to BCD
uint8_t stringToBcd(void) {
    const uint8_t firstNibble  = (line[0] - '0') & 0xF;
    const uint8_t secondNibble = (line[1] - '0') & 0xF;
    return (firstNibble << 4) + secondNibble;
}

// Read a 2-digit setting from user
uint8_t readSetting(void) {
    line[0] = 0;   
    line[8] = 255; 
    line[9] = 0;   

    while (true) {
        fgets(line, 9, stdin);

        if (strlen(line) != 3) { // includes line terminator
            printf("\r\nBad value. Please enter 2 digits only.\r\n");
            continue;
        }

        if (line[0] < '0' || line[0] > '9' || line[1] < '0' || line[1] > '9') {
            printf("\r\nBad value. Please enter digits 0-9 only.\r\n");
            continue;
        }

        break;
    }

    return stringToBcd();
}

// Calculate time difference in seconds
long timeDifferenceInSeconds(rtcDateTime *start, rtcDateTime *end) {
    long startSeconds = start->second + (start->minute * 60) + (start->hour * 3600);
    long endSeconds = end->second + (end->minute * 60) + (end->hour * 3600);
    
    // Handle day boundary crossing
    if (endSeconds < startSeconds) {
        endSeconds += 24 * 3600; // Add 24 hours worth of seconds
    }
    
    return endSeconds - startSeconds;
}

// Display current time
void displayTime(rtcDateTime *dt) {
    printf("%02d-%02d-%02d %02d:%02d:%02d", 
           dt->year, dt->month, dt->day, 
           dt->hour, dt->minute, dt->second);
}

// Set RTC time
void setTime(void) {
    printf("\r\nSet RTC time...\r\n");
    printf("Enter values in 2-digit format (YY MM DD HH MM SS)\r\n");

    printf("Year (00-99): ");
    datetime.year = readSetting();

    printf("\r\nMonth (01-12): ");
    datetime.month = readSetting();

    printf("\r\nDay (01-31): ");
    datetime.day = readSetting();

    printf("\r\nHour (00-23): ");
    datetime.hour = readSetting();

    printf("\r\nMinute (00-59): ");
    datetime.minute = readSetting();

    printf("\r\nSecond (00-59): ");
    datetime.second = readSetting();

    rp5c01SetMode(0); // Disable timer
    rp5c01SetTime(&datetime);
    rp5c01SetMode(MD_TIME); // Enable timer

    printf("\r\nTime set successfully!\r\n");
}

// Perform calibration test
void performCalibration(void) {
    rtcDateTime startTime, endTime;
    long actualSeconds, expectedSeconds;
    double accuracy;
    
    printf("\r\n=== RTC Calibration Test ===\r\n");
    printf("This test will measure RTC accuracy over %d minutes.\r\n", CALIB_DURATION_MINUTES);
    printf("Press any key to start, or ESC to cancel...\r\n");
    
    char key = 0;
    while ((key = cRawIo()) == 0) { }
    
    if (key == 27) { // ESC key
        printf("Calibration cancelled.\r\n");
        return;
    }
    
    // Get start time
    rp5c01GetTime(&startTime);
    convertFromBcd(&startTime);
    
    printf("\r\nCalibration started at: ");
    displayTime(&startTime);
    printf("\r\n");
    
    printf("Waiting %d minutes", CALIB_DURATION_MINUTES);
    
    // Wait for specified duration with progress indicator
    for (int minute = 0; minute < CALIB_DURATION_MINUTES; minute++) {
        for (int second = 0; second < 60; second++) {
            // Wait approximately 1 second (basic delay loop)
            for (int i = 0; i < 50000; i++) {
                // Check if user wants to abort
                if (cRawIo() == 27) { // ESC
                    printf("\r\nCalibration aborted by user.\r\n");
                    return;
                }
            }
            
            if (second % 10 == 0) {
                printf(".");  // Progress indicator
            }
        }
        printf(" %d", minute + 1);
    }
    
    // Get end time
    rp5c01GetTime(&endTime);
    convertFromBcd(&endTime);
    
    printf("\r\nCalibration ended at: ");
    displayTime(&endTime);
    printf("\r\n");
    
    // Calculate results
    expectedSeconds = CALIB_DURATION_MINUTES * 60;
    actualSeconds = timeDifferenceInSeconds(&startTime, &endTime);
    
    printf("\r\n=== Calibration Results ===\r\n");
    printf("Expected duration: %ld seconds\r\n", expectedSeconds);
    printf("Measured duration: %ld seconds\r\n", actualSeconds);
    printf("Difference: %ld seconds\r\n", actualSeconds - expectedSeconds);
    
    if (actualSeconds > 0) {
        accuracy = ((double)expectedSeconds / (double)actualSeconds) * 100.0;
        printf("RTC Accuracy: %.2f%%\r\n", accuracy);
        
        if (abs(actualSeconds - expectedSeconds) <= RTC_ACCURACY_THRESHOLD) {
            printf("Result: RTC is ACCURATE (within %d second tolerance)\r\n", RTC_ACCURACY_THRESHOLD);
        } else if (actualSeconds > expectedSeconds) {
            printf("Result: RTC is SLOW by %ld seconds\r\n", actualSeconds - expectedSeconds);
        } else {
            printf("Result: RTC is FAST by %ld seconds\r\n", expectedSeconds - actualSeconds);
        }
    }
    
    printf("\r\nPress any key to continue...\r\n");
    while (cRawIo() == 0) { }
}

// Test RTC functionality
void testRtc(void) {
    printf("\r\n=== RTC Hardware Test ===\r\n");
    
    // Test NVRAM write/read
    const uint8_t testValue = 0xA5;
    const uint8_t errorCode = rp5c01SetByte(0, testValue);
    
    if (errorCode) {
        printf("NVRAM write failed (error code: %02X)\r\n", errorCode);
        return;
    }
    
    const uint16_t readData = rp5c01GetByte(0);
    const uint8_t readValue = readData & 0xFF;
    const uint8_t readError = (readData >> 8) & 0xFF;
    
    if (readError) {
        printf("NVRAM read failed (error code: %02X)\r\n", readError);
        return;
    }
    
    if (readValue == testValue) {
        printf("NVRAM test: PASSED\r\n");
    } else {
        printf("NVRAM test: FAILED (wrote 0x%02X, read 0x%02X)\r\n", testValue, readValue);
        return;
    }
    
    // Test time read
    rp5c01GetTime(&datetime);
    convertFromBcd(&datetime);
    
    printf("Current time: ");
    displayTime(&datetime);
    printf("\r\n");
    
    printf("RTC hardware test completed successfully.\r\n");
    printf("Press any key to continue...\r\n");
    while (cRawIo() == 0) { }
}

// Display help
void showHelp(void) {
    printf("\r\n=== RTC Calibration Utility Help ===\r\n");
    printf("Commands:\r\n");
    printf("  T - Display current time\r\n");
    printf("  S - Set RTC time\r\n");
    printf("  C - Perform calibration test\r\n");
    printf("  H - Hardware test\r\n");
    printf("  ? - Show this help\r\n");
    printf("  Q - Quit program\r\n");
    printf("\r\nThis utility is designed for Ed Brindley's RTC card Rev1\r\n");
    printf("for the RC2014 computer system.\r\n");
    printf("\r\nPress any key to continue...\r\n");
    while (cRawIo() == 0) { }
}

void main(void) {
    char command;
    
    printf("RTC Calibration Utility v1.0\r\n");
    printf("For RC2014 with Ed Brindley's RTC Card Rev1\r\n");
    printf("========================================\r\n");

    // Detect RTC hardware
    const uint8_t detected = rp5c01Detect();
    
    if (!detected) {
        printf("ERROR: RP5C01 RTC not detected!\r\n");
        printf("Please check:\r\n");
        printf("- RTC card is properly installed\r\n");
        printf("- Card is configured for I/O ports B4h/B5h\r\n");
        printf("- Backup battery is connected\r\n");
        exit(1);
    }
    
    printf("RP5C01 RTC detected and ready.\r\n");
    
    // Initialize RTC
    rp5c01TestMode(0);           // Clear test modes
    rp5c01SetHourMode(HR_MD_24); // 24-hour format
    rp5c01SetMode(MD_TIME);      // Enable timer
    
    // Main menu loop
    while (true) {
        printf("\r\n--- Main Menu ---\r\n");
        printf("T)ime  S)et  C)alibrate  H)ardware test  ?)Help  Q)uit\r\n");
        printf("Command: ");
        
        // Wait for command
        command = 0;
        while ((command = cRawIo()) == 0) { }
        printf("%c\r\n", command);
        
        switch (command) {
            case 'T':
            case 't':
                rp5c01GetTime(&datetime);
                convertFromBcd(&datetime);
                printf("Current time: ");
                displayTime(&datetime);
                printf("\r\n");
                break;
                
            case 'S':
            case 's':
                setTime();
                break;
                
            case 'C':
            case 'c':
                performCalibration();
                break;
                
            case 'H':
            case 'h':
                testRtc();
                break;
                
            case '?':
                showHelp();
                break;
                
            case 'Q':
            case 'q':
                printf("Goodbye!\r\n");
                exit(0);
                break;
                
            default:
                printf("Unknown command. Press '?' for help.\r\n");
                break;
        }
    }
}
