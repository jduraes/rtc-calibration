#include "rtc.h"

void cpm_conout(char ch) {
    __asm
        ld c, 2
        ld e, l    ; Parameter is in L, not A
        call 5
    __endasm;
}

void cpm_print(const char *str) {
    while (*str) {
        cpm_conout(*str++);
    }
}

void print_hex(unsigned char value) {
    static char hex[] = "0123456789ABCDEF";
    cpm_conout(hex[(value >> 4) & 0xF]);
    cpm_conout(hex[value & 0xF]);
}

int main() {
    int result;
    RTC_Time time;
    int i;
    
    cpm_print("RTC DEBUG v0.1.5\r\n");
    
    // Test 0: Query HBIOS for RTC unit count
    cpm_print("Test 0: Query HBIOS for RTC count\r\n");
    __asm
        ld b, 0xF8      ; HBIOS SYSGET function
        ld c, 0x20      ; BF_SYSGET_RTCCNT
        rst 08
        ld l, a         ; Return result in HL for C
        ld h, 0
    __endasm;
    cpm_print("RTC unit count result: 0x");
    print_hex(result);
    cpm_print("\r\n");
    
    // Test 1: Direct HBIOS call
    cpm_print("Test 1: Direct HBIOS test call\r\n");
    result = hbios_rtc_test();
    cpm_print("HBIOS direct result: 0x");
    print_hex(result);
    cpm_print("\r\n");
    
    // Test 2: RTC detection
    cpm_print("Test 2: RTC detection\r\n");
    result = hbios_rtc_detect();
    cpm_print("RTC detect result: ");
    if (result) {
        cpm_print("RTC DETECTED\r\n");
    } else {
        cpm_print("RTC NOT DETECTED\r\n");
    }
    
    // Test 3: Get time (even if detection failed)
    cpm_print("Test 3: Attempt to get time\r\n");
    result = hbios_rtc_get_time(&time);
    cpm_print("Get time result: 0x");
    print_hex(result);
    cpm_print("\r\n");
    
    // Show raw buffer contents regardless of success/failure
    cpm_print("Raw RTC buffer contents: ");
    for (i = 0; i < 6; i++) {
        print_hex(((unsigned char*)&time)[i]);
        cpm_print(" ");
    }
    cpm_print("\r\n");
    
    cpm_print("Program complete.\r\n");
    return 0;
}
