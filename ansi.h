#ifndef ANSI_H
#define ANSI_H

// ANSI capability detection results
typedef enum {
    ANSI_UNKNOWN = 0,
    ANSI_NOT_SUPPORTED = 1,
    ANSI_SUPPORTED = 2,
    ANSI_FULL_SUPPORT = 3
} ansi_capability_t;

// ANSI colors
typedef enum {
    ANSI_BLACK = 0,
    ANSI_RED = 1,
    ANSI_GREEN = 2,
    ANSI_YELLOW = 3,
    ANSI_BLUE = 4,
    ANSI_MAGENTA = 5,
    ANSI_CYAN = 6,
    ANSI_WHITE = 7,
    ANSI_BRIGHT_BLACK = 8,
    ANSI_BRIGHT_RED = 9,
    ANSI_BRIGHT_GREEN = 10,
    ANSI_BRIGHT_YELLOW = 11,
    ANSI_BRIGHT_BLUE = 12,
    ANSI_BRIGHT_MAGENTA = 13,
    ANSI_BRIGHT_CYAN = 14,
    ANSI_BRIGHT_WHITE = 15
} ansi_color_t;

// Function prototypes

// Detection functions
ansi_capability_t ansi_detect_capability(void);
int ansi_test_cursor_position_report(void);
int ansi_test_device_attributes(void);

// Basic ANSI output functions
void ansi_clear_screen(void);
void ansi_home_cursor(void);
void ansi_goto_xy(int x, int y);
void ansi_clear_line(void);
void ansi_clear_to_eol(void);

// Color functions
void ansi_set_fg_color(ansi_color_t color);
void ansi_set_bg_color(ansi_color_t color);
void ansi_reset_colors(void);
void ansi_set_bold(void);
void ansi_set_dim(void);
void ansi_set_underline(void);
void ansi_reset_attributes(void);

// Cursor functions
void ansi_cursor_up(int lines);
void ansi_cursor_down(int lines);
void ansi_cursor_left(int cols); 
void ansi_cursor_right(int cols);
void ansi_save_cursor(void);
void ansi_restore_cursor(void);
void ansi_hide_cursor(void);
void ansi_show_cursor(void);

// Box drawing functions (for nice interfaces)
void ansi_draw_box(int x, int y, int width, int height);
void ansi_draw_horizontal_line(int x, int y, int length);
void ansi_draw_vertical_line(int x, int y, int length);

// Global state
extern ansi_capability_t g_ansi_capability;

#endif // ANSI_H
