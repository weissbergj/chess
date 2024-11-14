#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "uart.h"

void draw_pixel(int x, int y, color_t c) {
    unsigned int per_row = fb_get_width();
    unsigned int (*im)[per_row] = fb_get_draw_buffer();
    im[y][x] = c;
}

void draw_hline(int y, color_t c) {
    for (int x = 0; x < fb_get_width(); x++) {
        draw_pixel(x, y, c);
    }
}

void draw_vline(int x, color_t c) {
    for (int y = 0; y < fb_get_height(); y++) {
        draw_pixel(x, y, c);
    }
}

void draw_rectangle(int x, int y, int width, int height, color_t c) {
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            draw_pixel(i, j, c);
        }
    }
}

void main(void)  {
    uart_init();
    fb_init(800, 600, FB_SINGLEBUFFER); // using fb module from libmango

    const int square_size = 25;

    for (int y = 0; y < fb_get_height(); y += square_size) {
        for (int x = 0; x < fb_get_width(); x += square_size) {
            color_t color = ((x/square_size + y/square_size) % 2 == 0) ? 0xFFFF0000 : 0xFFFFFF00; // RED / YELLOW
            draw_rectangle(x, y, square_size, square_size, color);
        }
    }
    printf("Hit any key to quit: ");
    uart_getchar();
    printf("\nCompleted %s\n", __FILE__);
}
