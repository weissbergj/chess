/* File: console.c
 * ---------------
 * Text console
 */
#include "console.h"
#include "gl.h"
#include "malloc.h"  // CS107E version for malloc
#include "strings.h" // CS107E version for string operations
#include "printf.h"  // CS107E version for va_list and vsnprintf

// Console state
static struct {
    color_t background_color, foreground_color;
    int line_height, num_rows, num_cols, cursor_x, cursor_y;
    char **line_contents;
} console;

void console_init(int num_rows, int num_cols, color_t fg_color, color_t bg_color) {
    const int LINE_SPACING = 5;
    gl_init(num_cols * gl_get_char_width(), num_rows * (gl_get_char_height() + LINE_SPACING), GL_DOUBLEBUFFER);
    console = (typeof(console)){
        .line_height = gl_get_char_height() + LINE_SPACING,
        .foreground_color = fg_color,
        .background_color = bg_color,
        .num_rows = num_rows,
        .num_cols = num_cols
    };

    if (!(console.line_contents = malloc(num_rows * sizeof(char*)))) return;
    
    for (int row = 0; row < num_rows; row++) {
        if (!(console.line_contents[row] = malloc(num_cols + 1))) {
            while (row--) free(console.line_contents[row]);
            free(console.line_contents);
            return;
        }
        console.line_contents[row][0] = '\0';
    }
    console_clear();
}

void console_clear(void) {
    gl_clear(console.background_color);
    for (int row = 0; row < console.num_rows; row++) {
        console.line_contents[row][0] = '\0';
    }
    console.cursor_x = console.cursor_y = 0;
    gl_swap_buffer();
}

static void scroll_up(void) {
    for (int row = 0; row < console.num_rows - 1; row++) {
        memcpy(console.line_contents[row], console.line_contents[row + 1], console.num_cols + 1);
    }
    console.line_contents[console.num_rows - 1][0] = '\0';
}

static void process_char(char character) {
    if (character == '\b' && console.cursor_x > 0) {
        console.line_contents[console.cursor_y][--console.cursor_x] = '\0';
    } else if (character == '\n') {
        console.cursor_x = 0;
        if (++console.cursor_y >= console.num_rows) {
            scroll_up();
            console.cursor_y = console.num_rows - 1;
        }
    } else if (character == '\f') {
        console_clear();
    } else {
        if (console.cursor_x >= console.num_cols) {
            console.cursor_x = 0;
            if (++console.cursor_y >= console.num_rows) {
                scroll_up();
                console.cursor_y = console.num_rows - 1;
            }
        }
        console.line_contents[console.cursor_y][console.cursor_x++] = character;
        console.line_contents[console.cursor_y][console.cursor_x] = '\0';
    }
}

int console_printf(const char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    int chars_written = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    for (int i = 0; i < chars_written; i++) {
        process_char(buffer[i]);
    }
    
    gl_clear(console.background_color);
    for (int row = 0; row < console.num_rows; row++) {
        gl_draw_string(0, row * console.line_height, console.line_contents[row], console.foreground_color);
    }
    gl_swap_buffer();
    
    return chars_written;
}