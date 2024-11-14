/* File speed.c
 * ------------
 * Simple main program with timing code to measure
 * performance of different implementations of redraw.
 */

#include "gl.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"

// -------------------------------------------------------------------
// Time Trial Helpers
// -------------------------------------------------------------------


static void wait_for_user(const char *msg) {
    printf("%s: ", msg);
    char ch = uart_getchar();
    printf("%c\n", ch);
}

static unsigned long _time_trial(void (*fn)(color_t c)) {
    static int nrefresh = 0;
    color_t cycle[3] = {GL_RED, GL_WHITE, GL_BLUE};
    color_t c = cycle[nrefresh++ % 3];

    gl_clear(0xff555555);
    wait_for_user("type any key to start");
    unsigned long start = timer_get_ticks();
    fn(c);
    unsigned long elapsed = timer_get_ticks() - start;
    return elapsed;
}

#define TIME_TRIAL(fn) do {           \
    printf("Will run " #fn "... ");    \
    printf("took %ld ticks\n", _time_trial(fn)); \
} while (0)


// -------------------------------------------------------------------
// Baseline redraw0, correct implementation but pokey
// -------------------------------------------------------------------

static void redraw0(color_t c) {
    for (int y = 0; y < gl_get_height(); y++) {
        for (int x = 0; x < gl_get_width(); x++) {
            gl_draw_pixel(x, y, c);
        }
    }
}

// -------------------------------------------------------------------
// redraw1, only call getters once outside loop
// -------------------------------------------------------------------

static void redraw1(color_t c) {
    int h = gl_get_height();
    int w = gl_get_width();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            gl_draw_pixel(x, y, c);
        }
    }
}

// -------------------------------------------------------------------
// Optimize by using direct framebuffer access
// -------------------------------------------------------------------

static void redraw2(color_t c) {
    color_t *fb = fb_get_draw_buffer();
    int total_pixels = gl_get_width() * gl_get_height();
    
    for (int i = 0; i < total_pixels; i++) {
        fb[i] = c;
    }
}

// -------------------------------------------------------------------
// Optimize with word-sized writes (assuming 32-bit color)
// -------------------------------------------------------------------

static void redraw3(color_t c) {
    uint32_t *fb = (uint32_t *)fb_get_draw_buffer();
    int total_words = (gl_get_width() * gl_get_height());
    
    for (int i = 0; i < total_words; i += 4) {
        fb[i] = c;
        fb[i+1] = c;
        fb[i+2] = c;
        fb[i+3] = c;
    }
}

// -------------------------------------------------------------------
// Optimize with loop unrolling and aligned access
// -------------------------------------------------------------------

static void redraw4(color_t c) {
    uint64_t *fb = (uint64_t *)fb_get_draw_buffer();
    int total_words = (gl_get_width() * gl_get_height()) / 2;
    uint64_t double_color = ((uint64_t)c << 32) | c;
    
    for (int i = 0; i < total_words; i += 8) {
        fb[i] = double_color;
        fb[i+1] = double_color;
        fb[i+2] = double_color;
        fb[i+3] = double_color;
        fb[i+4] = double_color;
        fb[i+5] = double_color;
        fb[i+6] = double_color;
        fb[i+7] = double_color;
    }
}

// -------------------------------------------------------------------
// Optimize with DMA and cache alignment
// -------------------------------------------------------------------

static void redraw5(color_t c) {
    // 64-byte cache line alignment?
    uint64_t *fb = (uint64_t *)__builtin_assume_aligned(fb_get_draw_buffer(), 64);
    int total_words = (gl_get_width() * gl_get_height()) / 2;
    uint64_t double_color = ((uint64_t)c << 32) | c;
    
    #define UNROLL_SIZE 16
    for (int i = 0; i < total_words; i += UNROLL_SIZE) {
        __builtin_prefetch(&fb[i + UNROLL_SIZE], 1);
        fb[i+0] = double_color;
        fb[i+1] = double_color;
        fb[i+2] = double_color;
        fb[i+3] = double_color;
        fb[i+4] = double_color;
        fb[i+5] = double_color;
        fb[i+6] = double_color;
        fb[i+7] = double_color;
        fb[i+8] = double_color;
        fb[i+9] = double_color;
        fb[i+10] = double_color;
        fb[i+11] = double_color;
        fb[i+12] = double_color;
        fb[i+13] = double_color;
        fb[i+14] = double_color;
        fb[i+15] = double_color;
    }
}

void main(void)  {
    timer_init();
    uart_init();
    gl_init(1280, 720, GL_SINGLEBUFFER);

    printf("\nStarting time trials now.\n");

    TIME_TRIAL(redraw0);
    TIME_TRIAL(redraw1);
    TIME_TRIAL(redraw2);
    TIME_TRIAL(redraw3);
    TIME_TRIAL(redraw4);
    TIME_TRIAL(redraw5);

    printf("\nAll done with time trials.\n");
    wait_for_user("type any key to exit");
}
