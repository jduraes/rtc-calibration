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

unsigned char readDigit(void) {
    char ch;
    while (1) {
        ch = cRawIo();
        if (ch >= '0' && ch <= '9') {
            printChar(ch);
            return ch - '0';
        }
    }
}

unsigned char readTwoDigits(void) {
    unsigned char tens, ones;
    tens = readDigit();
    ones = readDigit();
    return tens * 10 + ones;
}

// Set RTC time
void setTime(void) {
    printStr("\r\nSet RTC time...\r\n");
    printStr("Enter YY MM DD HH MM SS (2 digits each)\r\n");

    printStr("Year: ");
    datetime.year = readTwoDigits();
    printStr("\r\n");

    printStr("Month: ");
    datetime.month = readTwoDigits();
    printStr("\r\n");

    printStr("Day: ");
    datetime.date = readTwoDigits();
    printStr("\r\n");

    printStr("Hour: ");
    datetime.hour = readTwoDigits();
    printStr("\r\n");

    printStr("Minute: ");
    datetime.minute = readTwoDigits();
    printStr("\r\n");

    printStr("Second: ");
    datetime.second = readTwoDigits();
    printStr("\r\n");

    convertToBcd(&datetime);
    if (hbios_rtc_set_time(&datetime) == 0) {
        printStr("Time set successfully!\r\n");
    } else {
        printStr("Error setting time!\r\n");
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
    RTC_Time test_time = {0, 0, 0, 0, 0, 0}; // Fresh local variable
    
    printStr("\r\n=== RTC Hardware Test ===\r\n");
    
    if (!hbios_rtc_detect()) {
        printStr("RTC not detected via HBIOS\r\n");
        return;
    }
    printStr("RTC detected via HBIOS\r\n");
    
    // Test HBIOS call directly
    int hbios_err = hbios_rtc_test();
    printStr("HBIOS: ");
    printHex((unsigned char)hbios_err);
    printStr("\r\n");
    
    // Test our wrapper function with fresh variable 
    int result = hbios_rtc_get_time(&test_time);
    printStr("Wrapper: ");
    printHex((unsigned char)result);
    printStr(" (full: ");
    printHex((unsigned char)(result >> 8));
    printHex((unsigned char)result);
    printStr(")\r\n");
    
    // Accept 0xB8 as valid since data is good
    if (result == 0 || result == 0xB8) {
        printStr("Raw HBIOS data (BCD): ");
        printHex(test_time.second); printChar(' ');
        printHex(test_time.minute); printChar(' ');
        printHex(test_time.hour); printChar(' ');
        printHex(test_time.date); printChar(' ');
        printHex(test_time.month); printChar(' ');
        printHex(test_time.year);
        printStr("\r\n");
        
        convertFromBcd(&test_time);
        printStr("Current RTC time: ");
        printDateTime(&test_time);
        printStr("\r\n");
    } else {
        printStr("Error reading RTC time\r\n");
    }
    
    printStr("RTC hardware test completed.\r\n");
    printStr("Press any key to continue...\r\n");
    while (cRawIo() == 0) { }
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
    printStr("Press any key to continue...\r\n");
    while (cRawIo() == 0) { }
}

void main(void) {
    char command;
    int result;
    
    printStr("RTC Calibration Utility v0.1.8 (HBIOS)\r\n");
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
