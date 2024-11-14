#include "font.h"
#include "printf.h"
#include "uart.h"

static void print_glyph(char ch) {
    uint8_t buf[font_get_glyph_size()];
    font_get_glyph(ch, buf, sizeof(buf));

    // Cast buf to a 2D array pointer of the correct dimensions
    uint8_t (*img)[font_get_glyph_width()] = (uint8_t (*)[font_get_glyph_width()]) buf;

    // TODO: declare a variable `img` of proper type to point
    // to a two-dimensional array of glyph width/height
    // and initialize img to point to buf
    // after your addition, code below will print the glyph as
    // "ascii art" using # and space characters
    for (int y = 0; y < font_get_glyph_height(); y++) {
        for (int x = 0; x < font_get_glyph_width(); x++) {
            printf("%c", img[y][x] == 0xff ? '#' : ' ');
        }
        printf("\n");
    }
}

void main(void) {
    uart_init();

    print_glyph('C');
    print_glyph('S');
    print_glyph('!');
}
