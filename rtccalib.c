#include "cpm.h"
#include "rtc.h"

// Function declarations
void printLong(unsigned long num);

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

// Add seconds and round to next 10-second mark
void adjustTimeRounded(RTC_Time *time, int seconds) {
    int total_seconds = time->second + seconds;
    int total_minutes = time->minute;
    int total_hours = time->hour;
    
    // Handle seconds overflow/underflow
    while (total_seconds >= 60) {
        total_seconds -= 60;
        total_minutes++;
    }
    while (total_seconds < 0) {
        total_seconds += 60;
        total_minutes--;
    }
    
    // Round to next 10-second mark
    if (seconds > 0) {
        // Round up to next decade
        total_seconds = ((total_seconds + 9) / 10) * 10;
        if (total_seconds >= 60) {
            total_seconds = 0;
            total_minutes++;
        }
    } else if (seconds < 0) {
        // Round down to previous decade
        total_seconds = (total_seconds / 10) * 10;
    }
    
    // Handle minutes overflow/underflow
    while (total_minutes >= 60) {
        total_minutes -= 60;
        total_hours++;
    }
    while (total_minutes < 0) {
        total_minutes += 60;
        total_hours--;
    }
    
    // Handle hours overflow/underflow
    while (total_hours >= 24) {
        total_hours -= 24;
    }
    while (total_hours < 0) {
        total_hours += 24;
    }
    
    // Update only the time portion
    time->second = total_seconds;
    time->minute = total_minutes;
    time->hour = total_hours;
}

// Print only time portion (HH:MM:SS)
void printTimeOnly(RTC_Time *time) {
    printNum2(time->hour);
    printChar(':');
    printNum2(time->minute);
    printChar(':');
    printNum2(time->second);
}

// Interactive time setter with arrow key support
// Returns 1 if ESC pressed (abort), 0 if time set successfully
int interactiveTimeSet(RTC_Time *time) {
    char ch;
    int escape_seq = 0;
    
    printStr("\r\nUse UP/DOWN arrows to adjust time by 10 seconds (rounded)\r\n");
    printStr("Press ENTER to set this time, ESC to abort\r\n\r\n");
    
    while (1) {
        // Display current time (time only, not date)
        printStr("\rTime: ");
        printTimeOnly(time);
        printStr("     ");  // Extra spaces to clear any leftover characters
        
        // Wait for key input
        ch = cRawIo();
        if (ch == 0) continue;
        
        if (escape_seq == 0 && ch == 27) {  // ESC or start of arrow sequence
            escape_seq = 1;
            continue;
        }
        
        if (escape_seq == 1) {
            if (ch == '[') {
                escape_seq = 2;
                continue;
            } else {
                // Just ESC key, abort
                return 1;
            }
        }
        
        if (escape_seq == 2) {
            escape_seq = 0;
            if (ch == 'A') {  // UP arrow
                adjustTimeRounded(time, 10);  // Add 10 seconds, rounded
            } else if (ch == 'B') {  // DOWN arrow
                adjustTimeRounded(time, -10);  // Subtract 10 seconds, rounded
            }
            // No continue here - let it immediately redraw
        }
        
        // Reset escape sequence state for any other key
        escape_seq = 0;
        
        if (ch == 13 || ch == 10) {  // Enter key - set the time
            return 0;
        }
        
        if (ch == 27) {  // Direct ESC without sequence
            return 1;
        }
    }
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

// Set RTC date only
void setDate(void) {
    char dateBuffer[20];
    RTC_Time current_time;
    unsigned char day, month, year;
    
    printStr("\r\n=== Set RTC Date ===\r\n");
    
    // Get current RTC time to preserve time and use current date as default
    int rtc_result = hbios_rtc_get_time(&current_time);
    if (rtc_result == 0 || rtc_result == 0xB8) {
        convertFromBcd(&current_time);
        printStr("Current date: ");
        printNum2(current_time.date);
        printChar('/');
        printNum2(current_time.month);
        printStr("/20");
        printNum2(current_time.year);
        printStr("\r\n");
    }
    
    // Get date in dd/mm/yyyy format with default
    if (rtc_result == 0 || rtc_result == 0xB8) {
        printStr("New date (dd/mm/yyyy) [");
        printNum2(current_time.date);
        printChar('/');
        printNum2(current_time.month);
        printStr("/20");
        printNum2(current_time.year);
        printStr("]: ");
    } else {
        printStr("New date (dd/mm/yyyy): ");
    }
    
    if (readString(dateBuffer, sizeof(dateBuffer))) {
        printStr("Aborted\r\n");
        return;
    }
    
    // If empty, use current date
    if (dateBuffer[0] == '\0' && (rtc_result == 0 || rtc_result == 0xB8)) {
        day = current_time.date;
        month = current_time.month;
        year = current_time.year;
    } else {
        if (!parseDate(dateBuffer, &day, &month, &year)) {
            printStr("Invalid date format. Use dd/mm/yyyy\r\n");
            return;
        }
    }
    
    // Preserve current time, update date
    datetime.date = day;
    datetime.month = month;
    datetime.year = year;
    if (rtc_result == 0 || rtc_result == 0xB8) {
        datetime.hour = current_time.hour;
        datetime.minute = current_time.minute;
        datetime.second = current_time.second;
    } else {
        datetime.hour = 0;
        datetime.minute = 0;
        datetime.second = 0;
    }
    
    // Convert to BCD and set the RTC
    convertToBcd(&datetime);
    if (hbios_rtc_set_time(&datetime) == 0) {
        printStr("\r\nDate set successfully to: ");
        // Convert back to decimal for display
        convertFromBcd(&datetime);
        printNum2(datetime.date);
        printChar('/');
        printNum2(datetime.month);
        printStr("/20");
        printNum2(datetime.year);
        printStr("\r\n");
    } else {
        printStr("\r\nError setting RTC date!\r\n");
    }
}

// Set RTC time with interactive adjustment
void setTime(void) {
    RTC_Time current_time;
    
    printStr("\r\n=== Set RTC Time ===\r\n");
    
    // Get current RTC time to use as starting point
    int rtc_result = hbios_rtc_get_time(&current_time);
    if (rtc_result == 0 || rtc_result == 0xB8) {
        convertFromBcd(&current_time);
        printStr("Current time: ");
        printDateTime(&current_time);
        printStr("\r\n");
    } else {
        printStr("Cannot read current time, using defaults\r\n");
        current_time.hour = 12;
        current_time.minute = 0;
        current_time.second = 0;
        current_time.date = 1;
        current_time.month = 1;
        current_time.year = 24;
    }
    
    // Interactive time adjustment
    if (interactiveTimeSet(&current_time)) {
        printStr("\r\nAborted\r\n");
        return;
    }
    
    // Set the RTC with the adjusted time
    datetime = current_time;  // Copy the adjusted time
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
        
        // Limit loop count to 25,000 to prevent system slowdown with high capacitance
        if (loop_count >= 25000) {
            break;
        }
        
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
    long expected_loops = 5000;  // Expected loops per second for properly calibrated RTC
    
    printStr("\r\n=== RTC Calibration Mode ===\r\n");
    printStr("CPU Clock: 7,372,800 Hz\r\n");
    printStr("Expected loops per RTC second: ");
    printLong(expected_loops);
    printStr("\r\n\r\n");
    
    printStr("Instructions:\r\n");
    printStr("- Measures RTC timing accuracy against CPU clock\r\n");
    printStr("- Shows percentage deviation from expected timing\r\n");
    printStr("- Adjust capacitor value to get close to 0.00%\r\n");
    printStr("- Press ESC to stop\r\n\r\n");
    
    printStr("Starting calibration...\r\n");
    
    // Calibration loop
    while (1) {
        // Check for ESC key first
        key = cRawIo();
        if (key == 27) {
            printStr("\r\nCalibration stopped.\r\n");
            break;
        }
        
        // Measure RTC timing
        long loop_count = measureRtcTiming();
        
        if (loop_count == 0x8000) {
            printStr("\rError reading RTC - retrying...        ");
            continue;
        }
        
        // Calculate deviation percentage
        long diff = loop_count - expected_loops;
        long pct_100 = (diff * 10000) / expected_loops;  // Percentage * 100 for 2 decimal places
        
        // Display the calibration result
        printStr("\rRTC Calibration: ");
        if (pct_100 > 0) {
            printStr("FAST by ");
        } else if (pct_100 < 0) {
            printStr("SLOW by ");
        } else {
            printStr("IN SYNC ");
        }
        
        if (pct_100 != 0) {
            printPercentage(pct_100 < 0 ? -pct_100 : pct_100);
        } else {
            printStr("0.00%");
        }
        
        printStr("    ");
        
        // Small delay to make it readable
        for (int i = 0; i < 10000; i++) { /* brief pause */ }
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
    printStr("  S - Show current date/time\r\n");
    printStr("  D - Set RTC date\r\n");
    printStr("  T - Set RTC time (with arrow key adjustment, 10s increments)\r\n");
    printStr("  H - Hardware test\r\n");
    printStr("  C - Calibrate RTC speed\r\n");
    printStr("  ? - Show this help\r\n");
    printStr("  Q - Quit program\r\n");
    printStr("\r\nFor RC2014 with RomWBW HBIOS RTC support\r\n");
}

void main(void) {
    char command;
    int result;
    
    printStr("RTC Calibration Utility v0.5.0 (HBIOS)\r\n");
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
        printStr("S)how Date/Time - Set D)ate / T)ime - H)ardware Test - C)alibrate - ?)Help - Q)uit\r\n");
        printStr("Command: ");
        
        // Wait for command
        command = 0;
        while ((command = cRawIo()) == 0) { }
        printChar(command);
        printStr("\r\n");
        
        switch (command) {
            case 'S':
            case 's':
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
                
            case 'D':
            case 'd':
                setDate();
                break;
                
            case 'T':
            case 't':
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
