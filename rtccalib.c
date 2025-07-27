#include "cpm.h"
#include "rtc.h"

// Simple putchar implementation for CP/M
int putchar(int c) {
    __asm
        ld c, 2
        ld e, l    ; Parameter is in L, not A
        call 5
    __endasm;
    return c;
}

// Convert BCD to decimal
unsigned char bcdToDecimal(unsigned char bcd) { 
    unsigned char hi = (bcd & 0xF0) >> 4;
    unsigned char lo = bcd & 0x0F;
    if (hi > 9 || lo > 9) return 0; // Invalid BCD
    return hi * 10 + lo;
}

// Convert decimal to BCD
unsigned char decimalToBcd(unsigned char decimal) {
    if (decimal > 99) decimal = 99; // Clamp to valid range
    unsigned char hi = decimal / 10;
    unsigned char lo = decimal % 10;
    return (hi << 4) | lo;
}

// Convert datetime from BCD to decimal
void convertFromBcd(RTC_Time *datetime) {
    datetime->second = bcdToDecimal(datetime->second);
    datetime->minute = bcdToDecimal(datetime->minute);
    datetime->hour   = bcdToDecimal(datetime->hour);
    datetime->date   = bcdToDecimal(datetime->date);
    datetime->month  = bcdToDecimal(datetime->month);
    datetime->year   = bcdToDecimal(datetime->year);
}

// Convert datetime from decimal to BCD
void convertToBcd(RTC_Time *datetime) {
    datetime->second = decimalToBcd(datetime->second);
    datetime->minute = decimalToBcd(datetime->minute);
    datetime->hour   = decimalToBcd(datetime->hour);
    datetime->date   = decimalToBcd(datetime->date);
    datetime->month  = decimalToBcd(datetime->month);
    datetime->year   = decimalToBcd(datetime->year);
}

RTC_Time datetime;

// Simple print functions without stdio
void printStr(char *str) {
    while (*str) {
        putchar(*str++);
    }
}

void printChar(char ch) {
    putchar(ch);
}

void printNum(unsigned char num) {
    if (num >= 10) {
        printChar('0' + (num / 10));
    }
    printChar('0' + (num % 10));
}

void printNum2(unsigned char num) {
    // Always print two digits with leading zero
    printChar('0' + (num / 10));
    printChar('0' + (num % 10));
}

void printDateTime(RTC_Time *dt) {
    // Format: dd/mm/yyyy hh:mm:ss
    printNum2(dt->date);
    printChar('/');
    printNum2(dt->month);
    printChar('/');
    printChar('2');  // Fixed '2' for 20xx
    printChar('0');  // Fixed '0' for 20xx
    printNum2(dt->year);
    printChar(' ');
    printNum2(dt->hour);
    printChar(':');
    printNum2(dt->minute);
    printChar(':');
    printNum2(dt->second);
}

// Read a string with ESC abort capability
// Returns 1 if ESC pressed (abort), 0 if string completed
int readString(char *buffer, int maxLen) {
    int pos = 0;
    char ch;
    
    while (pos < maxLen - 1) {
        ch = cRawIo();
        if (ch == 0) continue;  // No key pressed
        
        if (ch == 27) {  // ESC key
            return 1;  // Abort
        }
        
        if (ch == 13 || ch == 10) {  // Enter key
            buffer[pos] = '\0';
            printStr("\r\n");
            return 0;  // Success
        }
        
        if (ch == 8 || ch == 127) {  // Backspace
            if (pos > 0) {
                pos--;
                printStr("\b \b");  // Backspace, space, backspace
            }
            continue;
        }
        
        // Accept printable characters
        if (ch >= 32 && ch <= 126) {
            buffer[pos] = ch;
            pos++;
            printChar(ch);
        }
    }
    
    buffer[pos] = '\0';
    return 0;
}

// Parse date string in dd/mm/yyyy format
// Returns 1 on success, 0 on error
int parseDate(char *dateStr, unsigned char *day, unsigned char *month, unsigned char *year) {
    int d, m, y;
    
    // Simple parsing: expect exactly dd/mm/yyyy format
    if (dateStr[2] != '/' || dateStr[5] != '/') return 0;
    if (dateStr[0] < '0' || dateStr[0] > '9') return 0;
    if (dateStr[1] < '0' || dateStr[1] > '9') return 0;
    if (dateStr[3] < '0' || dateStr[3] > '9') return 0;
    if (dateStr[4] < '0' || dateStr[4] > '9') return 0;
    if (dateStr[6] < '0' || dateStr[6] > '9') return 0;
    if (dateStr[7] < '0' || dateStr[7] > '9') return 0;
    if (dateStr[8] < '0' || dateStr[8] > '9') return 0;
    if (dateStr[9] < '0' || dateStr[9] > '9') return 0;
    if (dateStr[10] != '\0') return 0;  // Must be exactly 10 chars
    
    d = (dateStr[0] - '0') * 10 + (dateStr[1] - '0');
    m = (dateStr[3] - '0') * 10 + (dateStr[4] - '0');
    y = (dateStr[6] - '0') * 1000 + (dateStr[7] - '0') * 100 + 
        (dateStr[8] - '0') * 10 + (dateStr[9] - '0');
    
    // Basic validation
    if (d < 1 || d > 31) return 0;
    if (m < 1 || m > 12) return 0;
    if (y < 2000 || y > 2099) return 0;  // We only support 20xx years
    
    *day = d;
    *month = m;
    *year = y - 2000;  // Store as 2-digit year
    return 1;
}

// Parse time string in HH:MM:SS format
// Returns 1 on success, 0 on error
int parseTime(char *timeStr, unsigned char *hour, unsigned char *minute, unsigned char *second) {
    int h, m, s;
    
    // Simple parsing: expect exactly HH:MM:SS format
    if (timeStr[2] != ':' || timeStr[5] != ':') return 0;
    if (timeStr[0] < '0' || timeStr[0] > '9') return 0;
    if (timeStr[1] < '0' || timeStr[1] > '9') return 0;
    if (timeStr[3] < '0' || timeStr[3] > '9') return 0;
    if (timeStr[4] < '0' || timeStr[4] > '9') return 0;
    if (timeStr[6] < '0' || timeStr[6] > '9') return 0;
    if (timeStr[7] < '0' || timeStr[7] > '9') return 0;
    if (timeStr[8] != '\0') return 0;  // Must be exactly 8 chars
    
    h = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    m = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
    s = (timeStr[6] - '0') * 10 + (timeStr[7] - '0');
    
    // Basic validation
    if (h > 23) return 0;
    if (m > 59) return 0;
    if (s > 59) return 0;
    
    *hour = h;
    *minute = m;
    *second = s;
    return 1;
}

// Set RTC time with new format
void setTime(void) {
    char dateBuffer[20];
    char timeBuffer[20];
    unsigned char day, month, year, hour, minute, second;
    
    printStr("\r\n=== Set RTC Time ===\r\n");
    printStr("Enter date and time (ESC to abort)\r\n\r\n");
    
    // Get date in dd/mm/yyyy format
    printStr("Date (dd/mm/yyyy): ");
    if (readString(dateBuffer, sizeof(dateBuffer))) {
        printStr("Aborted\r\n");
        return;
    }
    
    if (!parseDate(dateBuffer, &day, &month, &year)) {
        printStr("Invalid date format. Use dd/mm/yyyy\r\n");
        return;
    }
    
    // Get time in HH:MM:SS format
    printStr("Time (HH:MM:SS): ");
    if (readString(timeBuffer, sizeof(timeBuffer))) {
        printStr("Aborted\r\n");
        return;
    }
    
    if (!parseTime(timeBuffer, &hour, &minute, &second)) {
        printStr("Invalid time format. Use HH:MM:SS\r\n");
        return;
    }
    
    // Set the datetime structure
    datetime.date = day;
    datetime.month = month;
    datetime.year = year;
    datetime.hour = hour;
    datetime.minute = minute;
    datetime.second = second;
    
    // Convert to BCD and set the RTC
    convertToBcd(&datetime);
    if (hbios_rtc_set_time(&datetime) == 0) {
        printStr("\r\nTime set successfully to: ");
        // Convert back to decimal for display
        convertFromBcd(&datetime);
        printDateTime(&datetime);
        printStr("\r\n");
    } else {
        printStr("\r\nError setting RTC time!\r\n");
    }
}

void printHex(unsigned char val) {
    unsigned char hi = (val >> 4) & 0x0F;
    unsigned char lo = val & 0x0F;
    printChar(hi < 10 ? '0' + hi : 'A' + hi - 10);
    printChar(lo < 10 ? '0' + lo : 'A' + lo - 10);
}

// Test RTC functionality
void testRtc(void) {
    RTC_Time test_time = {0, 0, 0, 0, 0, 0};
    
    printStr("\r\n=== RTC Hardware Test ===\r\n");
    
    if (!hbios_rtc_detect()) {
        printStr("RTC not detected via HBIOS\r\n");
        return;
    }
    
    printStr("RTC hardware detected successfully\r\n");
    
    int result = hbios_rtc_get_time(&test_time);
    if (result == 0 || result == 0xB8) {
        convertFromBcd(&test_time);
        printStr("Current RTC time: ");
        printDateTime(&test_time);
        printStr("\r\n");
        printStr("RTC hardware is functioning correctly\r\n");
    } else {
        printStr("Error reading RTC time\r\n");
    }
}

// Display help
void showHelp(void) {
    printStr("\r\n=== RTC Calibration Utility Help ===\r\n");
    printStr("Commands:\r\n");
    printStr("  T - Display current time\r\n");
    printStr("  S - Set RTC time\r\n");
    printStr("  H - Hardware test\r\n");
    printStr("  ? - Show this help\r\n");
    printStr("  Q - Quit program\r\n");
    printStr("\r\nFor RC2014 with RomWBW HBIOS RTC support\r\n");
}

void main(void) {
    char command;
    int result;
    
    printStr("RTC Calibration Utility v0.1.9 (HBIOS)\r\n");
    printStr("For RC2014 with RomWBW HBIOS RTC support\r\n");
    printStr("========================================\r\n");

    // Detect RTC hardware via HBIOS
    if (!hbios_rtc_detect()) {
        printStr("ERROR: RTC not available via HBIOS!\r\n");
        printStr("Please check:\r\n");
        printStr("- RTC hardware is properly configured in RomWBW\r\n");
        printStr("- RTC driver is loaded in HBIOS\r\n");
        printStr("- RTC hardware is functioning\r\n");
        return;
    }
    
    printStr("RTC detected via HBIOS and ready.\r\n");
    
    // Main menu loop
    while (1) {
        printStr("\r\n--- Main Menu ---\r\n");
        printStr("T)ime  S)et  H)ardware test  ?)Help  Q)uit\r\n");
        printStr("Command: ");
        
        // Wait for command
        command = 0;
        while ((command = cRawIo()) == 0) { }
        printChar(command);
        printStr("\r\n");
        
        switch (command) {
            case 'T':
            case 't':
                result = hbios_rtc_get_time(&datetime);
                if (result == 0 || result == 0xB8) {
                    convertFromBcd(&datetime);
                    printStr("Current time: ");
                    printDateTime(&datetime);
                    printStr("\r\n");
                } else {
                    printStr("Error reading RTC time\r\n");
                }
                break;
                
            case 'S':
            case 's':
                setTime();
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
                printStr("Goodbye!\r\n");
                return;
                
            default:
                printStr("Unknown command. Press '?' for help.\r\n");
                break;
        }
    }
}
