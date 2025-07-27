// Compare the two RTC functions directly
extern int hbios_rtc_test(void);
extern int hbios_rtc_get_time(void *time_struct);

typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char date;
    unsigned char month;
    unsigned char year;
} RTC_Time;

void cpm_print_char(char ch) {
    __asm
        ld c, 2
        ld e, l
        call 5
    __endasm;
}

void cpm_print(const char *str) {
    while (*str) {
        cpm_print_char(*str++);
    }
}

void print_hex(unsigned char value) {
    static char hex[] = "0123456789ABCDEF";
    cpm_print_char(hex[(value >> 4) & 0xF]);
    cpm_print_char(hex[value & 0xF]);
}

int main() {
    RTC_Time time;
    int result;
    int i;
    
    cpm_print("RTC Function Comparison Test\r\n");
    
    // Test 1: Direct HBIOS call
    cpm_print("1. hbios_rtc_test result: 0x");
    result = hbios_rtc_test();
    print_hex(result);
    cpm_print("\r\n");
    
    // Test 2: Wrapper function
    cpm_print("2. hbios_rtc_get_time result: 0x");
    result = hbios_rtc_get_time(&time);
    print_hex(result);
    cpm_print("\r\n");
    
    // Show the actual data retrieved
    cpm_print("3. Data from get_time: ");
    for (i = 0; i < 6; i++) {
        print_hex(((unsigned char*)&time)[i]);
        cpm_print(" ");
    }
    cpm_print("\r\n");
    
    cpm_print("Test complete.\r\n");
    return 0;
}
