#include "ansi.h"

extern int cpm_putchar(int c);
#define putchar cpm_putchar

void print_num(int num) {
    if (num >= 10) {
        print_num(num / 10);
    }
    putchar('0' + (num % 10));
}

ansi_capability_t g_ansi_capability = ANSI_UNKNOWN;

// Send ANSI escape code for Device Attributes (DA) and read response
int ansi_test_device_attributes(void) {
    // Send DA escape code: ESC [ c
    putchar(27);  // ESC
    putchar('[');
    putchar('c');
    
    // Implement response reading (stub)
    // Normally, read from serial or input buffer and check for expected response
    return 0;  // Return 0 indicating stub (replace with actual check)
}

// Send ANSI escape code for Cursor Position Report (CPR) and read response
int ansi_test_cursor_position_report(void) {
    // Send CPR escape code: ESC [ 6 n
    putchar(27);  // ESC
    putchar('[');
    putchar('6');
    putchar('n');
    
    // Implement response reading (stub)
    // Normally, read from serial or input buffer and check for expected response
    return 0;  // Return 0 indicating stub (replace with actual check)
}

ansi_capability_t ansi_detect_capability(void) {
    // Try DA detection first
    if (ansi_test_device_attributes() == 0) {
        g_ansi_capability = ANSI_SUPPORTED;
        return g_ansi_capability;
    }
    
    // Fallback to CPR detection
    if (ansi_test_cursor_position_report() == 0) {
        g_ansi_capability = ANSI_SUPPORTED;
        return g_ansi_capability;
    }
    
    // If neither worked, assume not supported
    g_ansi_capability = ANSI_NOT_SUPPORTED;
    return g_ansi_capability;
}

void ansi_clear_screen(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('2');
    putchar('J');
}

void ansi_home_cursor(void) {
    putchar(27);
    putchar('[');
    putchar('H');
}

void ansi_goto_xy(int x, int y) {
    putchar(27);  // ESC
    putchar('[');
    print_num(y);
    putchar(';');
    print_num(x);
    putchar('H');
}

void ansi_clear_line(void) {
    putchar(27);
    putchar('[');
    putchar('K');
}

void ansi_clear_to_eol(void) {
    putchar(27);
    putchar('[');
    putchar('K');
}

void ansi_set_fg_colour(ansi_colour_t colour) {
    putchar(27);  // ESC
    putchar('[');
    
    if (colour >= 8) {
        // Bright colours: ESC[1;3Xm format
        putchar('1');
        putchar(';');
        putchar('3');
        putchar('0' + (colour - 8));
    } else {
        // Normal colours: ESC[3Xm format (30-37)
        putchar('3');
        putchar('0' + colour);
    }
    putchar('m');
}

void ansi_set_bg_colour(ansi_colour_t colour) {
    putchar(27);  // ESC
    putchar('[');
    if (colour >= 8) {
        // Bright background colours (8-15): use ESC[10Xm format
        putchar('1');
        putchar('0');
        putchar('0' + (colour - 8));
    } else {
        // Normal background colours (0-7): use ESC[4Xm format (40-47)
        putchar('4');
        putchar('0' + colour);
    }
    putchar('m');
}

void ansi_reset_colours(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('0');
    putchar('m');
}

void ansi_set_bold(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('1');
    putchar('m');
}

void ansi_set_dim(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('2');
    putchar('m');
}

void ansi_set_underline(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('4');
    putchar('m');
}

void ansi_reset_attributes(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('0');
    putchar('m');
}

void ansi_cursor_up(int lines) {
    putchar(27);  // ESC
    putchar('[');
    print_num(lines);
    putchar('A');
}

void ansi_cursor_down(int lines) {
    putchar(27);  // ESC
    putchar('[');
    print_num(lines);
    putchar('B');
}

void ansi_cursor_right(int cols) {
    putchar(27);  // ESC
    putchar('[');
    print_num(cols);
    putchar('C');
}

void ansi_cursor_left(int cols) {
    putchar(27);  // ESC
    putchar('[');
    print_num(cols);
    putchar('D');
}

void ansi_save_cursor(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('s');
}

void ansi_restore_cursor(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('u');
}

void ansi_hide_cursor(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('?');
    putchar('2');
    putchar('5');
    putchar('l');
}

void ansi_show_cursor(void) {
    putchar(27);  // ESC
    putchar('[');
    putchar('?');
    putchar('2');
    putchar('5');
    putchar('h');
}

// Simple box drawing (ASCII fallback)
void ansi_draw_box(int x, int y, int width, int height) {
    int i;
    
    // Top border
    ansi_goto_xy(x, y);
    putchar('+');
    for (i = 1; i < width - 1; i++) {
        putchar('-');
    }
    putchar('+');
    
    // Side borders
    for (i = 1; i < height - 1; i++) {
        ansi_goto_xy(x, y + i);
        putchar('|');
        ansi_goto_xy(x + width - 1, y + i);
        putchar('|');
    }
    
    // Bottom border
    ansi_goto_xy(x, y + height - 1);
    putchar('+');
    for (i = 1; i < width - 1; i++) {
        putchar('-');
    }
    putchar('+');
}

void ansi_draw_horizontal_line(int x, int y, int length) {
    int i;
    ansi_goto_xy(x, y);
    for (i = 0; i < length; i++) {
        putchar('-');
    }
}

void ansi_draw_vertical_line(int x, int y, int length) {
    int i;
    for (i = 0; i < length; i++) {
        ansi_goto_xy(x, y + i);
        putchar('|');
    }
}

