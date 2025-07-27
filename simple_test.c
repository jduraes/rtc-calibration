// Very simple RTC test program
extern int hbios_rtc_test(void);

void cpm_exit(void) {
    __asm
        ld c, 0
        call 5
    __endasm;
}

void cpm_print_char(char ch) {
    __asm
        ld c, 2
        ld e, l    ; Parameter is in L, not A
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
    int result;
    
    cpm_print("Simple RTC Test\r\n");
    
    result = hbios_rtc_test();
    
    cpm_print("HBIOS result: 0x");
    print_hex(result);
    cpm_print("\r\n");
    
    cpm_exit();
    return 0;
}
