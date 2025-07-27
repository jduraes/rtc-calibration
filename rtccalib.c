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

// Enhanced string reading with arrow key support for time adjustment
// Returns: 0=success, 1=ESC abort, 2=use default
int readStringWithArrows(char *buffer, int maxLen, char *defaultStr, int *timeAdjust) {
    int pos = 0;
    char ch;
    int escapeSeq = 0;
    
    *timeAdjust = 0;  // Reset time adjustment
    
    // Show default value in brackets
    printStr("[");
    printStr(defaultStr);
    printStr("]: ");
    
    while (pos < maxLen - 1) {
        ch = cRawIo();
        if (ch == 0) continue;  // No key pressed
        
        // Handle escape sequences
        if (escapeSeq == 1) {
            if (ch == '[') {
                escapeSeq = 2;
                continue;
            } else {
                escapeSeq = 0;
                return 1;  // ESC without sequence = abort
            }
        }
        
        if (escapeSeq == 2) {
            escapeSeq = 0;
            if (ch == 'A') {  // Up arrow
                *timeAdjust = 5;  // +5 seconds
                printStr("[+5s] ");
                return 2;  // Use default with adjustment
            } else if (ch == 'B') {  // Down arrow
                *timeAdjust = -5;  // -5 seconds
                printStr("[-5s] ");
                return 2;  // Use default with adjustment
            }
            continue;
        }
        
        if (ch == 27) {  // ESC key
            escapeSeq = 1;
            continue;
        }
        
        if (ch == 13 || ch == 10) {  // Enter key
            if (pos == 0) {
                return 2;  // Use default (empty input)
            }
            buffer[pos] = '\0';
            printStr("\r\n");
            return 0;  // Success with user input
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

// Format time as HH:MM:SS string
void formatTime(char *buffer, unsigned char hour, unsigned char minute, unsigned char second) {
    buffer[0] = '0' + (hour / 10);
    buffer[1] = '0' + (hour % 10);
    buffer[2] = ':';
    buffer[3] = '0' + (minute / 10);
    buffer[4] = '0' + (minute % 10);
    buffer[5] = ':';
    buffer[6] = '0' + (second / 10);
    buffer[7] = '0' + (second % 10);
    buffer[8] = '\0';
}

// Format date as dd/mm/yyyy string
void formatDate(char *buffer, unsigned char day, unsigned char month, unsigned char year) {
    buffer[0] = '0' + (day / 10);
    buffer[1] = '0' + (day % 10);
    buffer[2] = '/';
    buffer[3] = '0' + (month / 10);
    buffer[4] = '0' + (month % 10);
    buffer[5] = '/';
    buffer[6] = '2';
    buffer[7] = '0';
    buffer[8] = '0' + ((year + 2000) / 10 % 10);
    buffer[9] = '0' + ((year + 2000) % 10);
    buffer[10] = '\0';
}

// Adjust time by seconds (can be positive or negative)
void adjustTime(unsigned char *hour, unsigned char *minute, unsigned char *second, int adjustment) {
    int totalSeconds = *hour * 3600 + *minute * 60 + *second + adjustment;
    
    // Handle day wraparound
    while (totalSeconds < 0) totalSeconds += 86400;  // Add 24 hours
    while (totalSeconds >= 86400) totalSeconds -= 86400;  // Subtract 24 hours
    
    *hour = totalSeconds / 3600;
    *minute = (totalSeconds % 3600) / 60;
    *second = totalSeconds % 60;
}

// Set RTC time with enhanced UI
void setTime(void) {
    char dateBuffer[20];
    char timeBuffer[20];
    char defaultDateBuffer[20];
    char defaultTimeBuffer[20];
    unsigned char day, month, year, hour, minute, second;
    int result, timeAdjust;
    
    printStr("\r\n=== Set RTC Time ===\r\n");
    printStr("Enhanced UI: Press Enter for default, Up/Down arrows for time ±5s\r\n");
    printStr("ESC to abort\r\n\r\n");
    
    // Get current RTC time for defaults
    result = hbios_rtc_get_time(&datetime);
    if (result != 0 && result != 0xB8) {
        printStr("Error reading current RTC time\r\n");
        return;
    }
    convertFromBcd(&datetime);
    
    // Format current date and time as defaults
    formatDate(defaultDateBuffer, datetime.date, datetime.month, datetime.year);
    formatTime(defaultTimeBuffer, datetime.hour, datetime.minute, datetime.second);
    
    // Get date with default
    printStr("Date (dd/mm/yyyy) ");
    result = readStringWithArrows(dateBuffer, sizeof(dateBuffer), defaultDateBuffer, &timeAdjust);
    
    if (result == 1) {  // ESC abort
        printStr("\r\nAborted\r\n");
        return;
    } else if (result == 2) {  // Use default
        day = datetime.date;
        month = datetime.month;
        year = datetime.year;
        printStr("\r\n");
    } else {  // Parse user input
        if (!parseDate(dateBuffer, &day, &month, &year)) {
            printStr("\r\nInvalid date format. Use dd/mm/yyyy\r\n");
            return;
        }
    }
    
    // Get time with default and arrow key support
    printStr("Time (HH:MM:SS) ");
    result = readStringWithArrows(timeBuffer, sizeof(timeBuffer), defaultTimeBuffer, &timeAdjust);
    
    if (result == 1) {  // ESC abort
        printStr("\r\nAborted\r\n");
        return;
    } else if (result == 2) {  // Use default or arrow adjustment
        hour = datetime.hour;
        minute = datetime.minute;
        second = datetime.second;
        
        // Apply arrow key adjustment if any
        if (timeAdjust != 0) {
            adjustTime(&hour, &minute, &second, timeAdjust);
        }
        printStr("\r\n");
    } else {  // Parse user input
        if (!parseTime(timeBuffer, &hour, &minute, &second)) {
            printStr("\r\nInvalid time format. Use HH:MM:SS\r\n");
            return;
        }
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


// Print a 32-bit number in decimal (non-recursive to avoid stack issues)
void printLong(unsigned long num) {
    char buffer[12];  // Enough for 32-bit number + null terminator
    int pos = 0;
    
    if (num == 0) {
        printChar('0');
        return;
    }
    
    // Build digits in reverse order
    while (num > 0) {
        buffer[pos++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Print in correct order
    while (pos > 0) {
        printChar(buffer[--pos]);
    }
}

// Print a 32-bit number with comma separators (non-recursive)
void printLongWithCommas(unsigned long num) {
    char buffer[16];  // Enough for number + commas
    int pos = 0;
    int digit_count = 0;
    
    if (num == 0) {
        printChar('0');
        return;
    }
    
    // Build digits in reverse order with commas
    while (num > 0) {
        if (digit_count > 0 && digit_count % 3 == 0) {
            buffer[pos++] = ',';
        }
        buffer[pos++] = '0' + (num % 10);
        num /= 10;
        digit_count++;
    }
    
    // Print in correct order
    while (pos > 0) {
        printChar(buffer[--pos]);
    }
}

// Print difference with sign
void printSignedDiff(long diff) {
    if (diff < 0) {
        printChar('-');
        printLong(-diff);
    } else if (diff > 0) {
        printChar('+');
        printLong(diff);
    } else {
        printChar('0');
    }
}

// Print percentage with 2 decimal places (multiplied by 100)
void printPercentage(long pct_100) {
    if (pct_100 < 0) {
        printChar('-');
        pct_100 = -pct_100;
    }
    
    unsigned long whole = pct_100 / 100;
    unsigned long frac = pct_100 % 100;
    
    printLong(whole);
    printChar('.');
    if (frac < 10) printChar('0');
    printLong(frac);
    printChar('%');
}

// Simple RTC timing measurement - avoid crashes by using minimal RTC calls
long measureRtcTiming(void) {
    RTC_Time start_time, current_time;
    unsigned long loop_count = 0;
    unsigned char start_second, current_second;
    int rtc_result;
    
    // Get initial RTC time
    rtc_result = hbios_rtc_get_time(&start_time);
    if (rtc_result != 0 && rtc_result != 0xB8) {
        return 0x8000;  // Error code
    }
    convertFromBcd(&start_time);
    start_second = start_time.second;
    
    // Wait for second to change to get clean boundary
    do {
        rtc_result = hbios_rtc_get_time(&current_time);
        if (rtc_result != 0 && rtc_result != 0xB8) {
            return 0x8000;  // Error code
        }
        convertFromBcd(&current_time);
        current_second = current_time.second;
    } while (current_second == start_second);
    
    // Now count loops for exactly one second
    start_second = current_second;
    do {
        loop_count++;
        
        // Check RTC every 5000 loops to avoid too many HBIOS calls but stay responsive
        if ((loop_count % 5000) == 0) {
            rtc_result = hbios_rtc_get_time(&current_time);
            if (rtc_result != 0 && rtc_result != 0xB8) {
                return 0x8000;  // Error code
            }
            convertFromBcd(&current_time);
            current_second = current_time.second;
        }
    } while (current_second == start_second);
    
    // For now, return the raw loop count for calibration
    // We'll use this to determine the correct expected value
    return (long)loop_count;
}

// RTC Calibration using CPU clock as reference
void calibrateRtc(void) {
    char key;
    
    printStr("\r\n=== RTC Calibration Mode ===\r\n");
    printStr("Using CPU clock as reference\r\n");
    printStr("CPU Clock: 7,372,800 Hz\r\n\r\n");
    
    printStr("This measures RTC timing accuracy by counting\r\n");
    printStr("CPU loops during one RTC second.\r\n\r\n");
    
    printStr("Instructions:\r\n");
    printStr("- Positive % = RTC running FAST\r\n");
    printStr("- Negative % = RTC running SLOW\r\n");
    printStr("- 0.00% = RTC perfectly in sync\r\n");
    printStr("- Press ESC to stop\r\n\r\n");
    
    printStr("Starting calibration...\r\n");
    
    // Calibration loop
    while (1) {
        // Check for ESC key
        key = cRawIo();
        if (key == 27) {
            printStr("\r\nCalibration stopped.\r\n");
            break;
        }
        
        printStr("\rMeasuring... ");
        
        // Measure RTC timing
        long pct_100 = measureRtcTiming();
        
        if (pct_100 == 0x8000) {
            printStr("\rError reading RTC          ");
            continue;
        }
        
        // Display raw loop count for baseline calibration
        printStr("\rLoop count: ");
        printLong(pct_100);  // This is actually the loop count now
        printStr(" loops/sec          ");
    }
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
    printStr("  C - Calibrate RTC speed\r\n");
    printStr("  ? - Show this help\r\n");
    printStr("  Q - Quit program\r\n");
    printStr("\r\nFor RC2014 with RomWBW HBIOS RTC support\r\n");
}

void main(void) {
    char command;
    int result;
    
    printStr("RTC Calibration Utility v0.3.0 (HBIOS)\r\n");
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
    printStr("T)ime  S)et  H)ardware test  C)alibrate  ?)Help  Q)uit\r\n");
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
                
            case 'C':
            case 'c':
                calibrateRtc();
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
