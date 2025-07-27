#include "cpm.h"

// Function prototype for putchar
int putchar(int c);

void printStr(char *str) {
    while (*str) {
        putchar(*str++);
    }
}

void printHex(unsigned char val) {
    char hex[] = "0123456789ABCDEF";
    putchar(hex[val >> 4]);
    putchar(hex[val & 0x0F]);
}

unsigned char val;

void main(void) {
    printStr("Port 0xC0 Test\r\n");
    printStr("==============\r\n");
    
    // Test reading from port 0xC0 directly
    for (int i = 0; i < 10; i++) {
        __asm
            ld c, 0xC0
            in a, (c)
            ld (_val), a
        __endasm;
        
        printStr("Port 0xC0: ");
        printHex(val);
        printStr("\r\n");
        
        // Small delay
        for (int j = 0; j < 1000; j++) {
            cRawIo();
        }
    }
    
    printStr("Done.\r\n");
}
