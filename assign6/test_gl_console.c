/* File: test_gl_console.c
 * -----------------------
 * ***** Test cases for console *****
 */
#include "assert.h"
#include "console.h"
#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"

extern void enable_fpu(void);

static void pause(const char *message) {
    if (message) printf("\n%s\n", message);
    printf("[PAUSED] type any key in tio to continue: ");
    int ch = uart_getchar();
    uart_putchar(ch);
    uart_putchar('\n');
}

static void test_fb(void) {
    const int SIZE = 500;
    fb_init(SIZE, SIZE, FB_SINGLEBUFFER); // init single buffer

    assert(fb_get_width() == SIZE);
    assert(fb_get_height() == SIZE);
    assert(fb_get_depth() == 4);

    void *cptr = fb_get_draw_buffer();
    assert(cptr != NULL);
    int nbytes = fb_get_width() * fb_get_height() * fb_get_depth();
    memset(cptr, 0x99, nbytes); // fill entire framebuffer with light gray pixels
    pause("Now displaying 500 x 500 screen of light gray pixels");

    fb_init(1280, 720, FB_DOUBLEBUFFER); // init double buffer
    cptr = fb_get_draw_buffer();
    nbytes =  fb_get_width() * fb_get_height() * fb_get_depth();
    memset(cptr, 0xff, nbytes); // fill one buffer with white pixels
    fb_swap_buffer();
    pause("Now displaying 1280 x 720 white pixels");

    cptr = fb_get_draw_buffer();
    memset(cptr, 0x33, nbytes); // fill other buffer with dark gray pixels
    fb_swap_buffer();
    pause("Now displaying 1280 x 720 dark gray pixels");

    for (int i = 0; i < 5; i++) {
        fb_swap_buffer();
        timer_delay_ms(250);
    }
}

static void test_gl(void) {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    // Double buffer mode, make sure you test single buffer too!
    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);
    assert(gl_get_height() == HEIGHT);
    assert(gl_get_width() == WIDTH);

    // Background is purple
    gl_clear(gl_color(0x55, 0, 0x55)); // create purple color

    // Draw green pixel in lower right
    gl_draw_pixel(WIDTH-10, HEIGHT-10, GL_GREEN);
    assert(gl_read_pixel(WIDTH-10, HEIGHT-10) == GL_GREEN);

    // Blue rectangle in center of screen
    gl_draw_rect(WIDTH/2 - 100, HEIGHT/2 - 50, 200, 100, GL_BLUE);

    // Single amber character
    gl_draw_char(60, 10, 'A', GL_AMBER);

    // Show buffer with drawn contents
    gl_swap_buffer();
    pause("Now displaying 1280 x 720, purple bg, single green pixel, blue center rect, amber letter A");
}

static void test_console(void) {
    console_init(25, 50, GL_CYAN, GL_INDIGO);
    pause("Now displaying console: 25 rows x 50 columns, bg indigo, fg cyan");

    // Line 1: Hello, world!
    console_printf("Hello, world!\n");

    // Add line 2: Happiness == CODING
    console_printf("Happiness");
    console_printf(" == ");
    console_printf("CODING\n");

    // Add 2 blank lines and line 5: I am Pi, hear me roar!
    console_printf("\n\nI am Pi, hear me v\b \broar!\n"); // typo, backspace, correction
    pause("Console printfs");

    // Clear all lines
    console_printf("\f");

    // Line 1: "Goodbye"
    console_printf("Goodbye!\n");
    pause("Console clear");
}

static void test_mandelbrot(void) {
    const int WIDTH = 1280;
    const int HEIGHT = 720;
    const int MAX_ITER = 100;
    
    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);
    
    const double MIN_REAL = -2.0;
    const double MAX_REAL = 0.5;
    const double MIN_IMAG = -1.25;
    const double MAX_IMAG = 1.25;
    
    typedef struct { double real; double imag; } Complex;
    
    double real_factor = (MAX_REAL - MIN_REAL) / (WIDTH - 1);
    double imag_factor = (MAX_IMAG - MIN_IMAG) / (HEIGHT - 1);
    
    for (int y = 0; y < HEIGHT; y++) {
        double c_imag = MIN_IMAG + y * imag_factor;
        for (int x = 0; x < WIDTH; x++) {
            double c_real = MIN_REAL + x * real_factor;
            Complex z = {0, 0};
            int iter = 0;
            
            while (z.real * z.real + z.imag * z.imag <= 4.0 && iter < MAX_ITER) {
                double z_real_temp = z.real * z.real - z.imag * z.imag + c_real;
                z.imag = 2 * z.real * z.imag + c_imag;
                z.real = z_real_temp;
                iter++;
            }
            
            color_t color = (iter == MAX_ITER) ? GL_BLACK : gl_color((iter * 5) % 256, (iter * 7) % 256, (iter * 11) % 256);
            gl_draw_pixel(x, y, color);
        }
    }
    
    gl_draw_string(10, 10, "Mandelbrot Set", GL_WHITE);
    gl_swap_buffer();
    pause("Mandelbrot set");
}

static void test_clipping(void) {
    gl_init(823, 617, GL_SINGLEBUFFER); // larger than 800x600
    gl_clear(GL_BLACK);

    gl_draw_pixel(-3, -2, GL_RED); // off-screen
    gl_draw_pixel(824, 618, GL_RED); // off-screen
    gl_draw_pixel(0, 0, GL_GREEN); // corner
    gl_draw_pixel(822, 616, GL_GREEN); // corner
    assert(gl_read_pixel(0, 0) == GL_GREEN);
    assert(gl_read_pixel(822, 616) == GL_GREEN);

    gl_draw_rect(-47, -53, 98, 103, GL_BLUE); // off-screen top-left
    gl_draw_rect(767, 548, 97, 102, GL_RED); // off-screen bottom-right
    assert(gl_read_pixel(0, 0) == GL_BLUE);
    assert(gl_read_pixel(822, 616) == GL_RED);

    pause("Clipping test");
}

static void test_text_drawing(void) {
    gl_init(823, 617, GL_DOUBLEBUFFER);
    gl_clear(GL_BLACK);

    gl_draw_string(12, 13, "Hello, Graphics!", GL_WHITE); // normal text
    gl_draw_string(-7, 11, "Clipped Left", GL_RED); // clipped left
    gl_draw_string(763, 13, "Clipped Right", GL_GREEN); // clipped right
    gl_draw_string(11, -4, "Clipped Top", GL_BLUE); // clipped top
    gl_draw_string(13, 607, "Clipped Bottom", GL_YELLOW); // clipped bottom

    gl_draw_char(822, 616, 'X', GL_CYAN); // mostly clipped
    gl_draw_char(-2, -3, 'Y', GL_MAGENTA); // fully clipped

    int y = 308;
    gl_draw_string(203, y, "Congratulations!", GL_GREEN);
    gl_draw_string(203, y + 23, "Your graphics library is working!", GL_CYAN);
    gl_draw_string(203, y + 42, "Time to celebrate! 🎉", GL_MAGENTA);

    gl_swap_buffer();
    pause("Text drawing and clipping");
}

static void test_console_wrap_scroll(void) {
    console_init(25, 50, GL_CYAN, GL_INDIGO);
    pause("Console wrap and scroll");

    console_printf("LALALALALA This is a very long line that should wrap jajajajajja to the next line automatically because alsk;jdf it's longer than 50 characters LALALALALA\n");
    pause("Line wrap");

    for (int i = 1; i <= 30; i++) {
        console_printf("Line %d: Testing console scrolling behavior\n", i);
        timer_delay_ms(100);
    }
    pause("Scrolling test");

    for (int i = 1; i <= 5; i++) {
        console_printf("Long line %d: LALALALALA This text should  wrap AND cause scrolling when it reaches is this longer? idk just adding just in case is it is it????the bottom of the console display area LALALALALA\n", i);
        timer_delay_ms(200);
    }
    pause("Wrap and scroll");
}

static void test_gl_getters_and_colors(void) { // did not print anything! FIX
    gl_init(823, 617, GL_SINGLEBUFFER);
    assert(gl_get_width() == 823);
    assert(gl_get_height() == 617);
    
    gl_init(1024, 768, GL_DOUBLEBUFFER);
    assert(gl_get_width() == 1024);
    assert(gl_get_height() == 768);

    color_t white = gl_color(255, 255, 255);
    color_t black = gl_color(0, 0, 0);
    color_t pure_red = gl_color(255, 0, 0);
    color_t pure_green = gl_color(0, 255, 0);
    color_t pure_blue = gl_color(0, 0, 255);
    color_t mixed = gl_color(123, 45, 67);

    gl_clear(black);
    gl_draw_rect(10, 10, 50, 50, white);
    gl_draw_rect(70, 10, 50, 50, pure_red);
    gl_draw_rect(130, 10, 50, 50, pure_green);
    gl_draw_rect(190, 10, 50, 50, pure_blue);
    gl_draw_rect(250, 10, 50, 50, mixed);

    assert(gl_read_pixel(35, 35) == white);
    assert(gl_read_pixel(95, 35) == pure_red);
    assert(gl_read_pixel(155, 35) == pure_green);
    assert(gl_read_pixel(215, 35) == pure_blue);
    assert(gl_read_pixel(275, 35) == mixed);

    gl_swap_buffer();
    pause("Color test");
}

static void test_rectangle_clipping_extended(void) {
    gl_init(823, 617, GL_SINGLEBUFFER);
    gl_clear(GL_BLACK);

    gl_draw_rect(-100, -100, 1023, 817, GL_PURPLE); // large rect

    gl_draw_rect(-25, 100, 50, 50, GL_RED); // left edge
    gl_draw_rect(798, 100, 50, 50, GL_GREEN); // right edge
    gl_draw_rect(100, -25, 50, 50, GL_BLUE); // top edge
    gl_draw_rect(100, 592, 50, 50, GL_YELLOW); // bottom edge

    gl_draw_rect(-25, -25, 50, 50, GL_CYAN); // top-left
    gl_draw_rect(798, -25, 50, 50, GL_MAGENTA); // top-right
    gl_draw_rect(-25, 592, 50, 50, GL_WHITE); // bottom-left
    gl_draw_rect(798, 592, 50, 50, GL_AMBER); // bottom-right

    gl_draw_rect(50, 50, 1, 100, GL_GREEN); // vertical line
    gl_draw_rect(50, 50, 100, 1, GL_RED); // horizontal line
    gl_draw_rect(200, 200, 2, 2, GL_BLUE); // small square

    assert(gl_read_pixel(0, 100) == GL_RED);
    assert(gl_read_pixel(822, 100) == GL_GREEN);
    assert(gl_read_pixel(100, 0) == GL_BLUE);
    assert(gl_read_pixel(100, 616) == GL_YELLOW);

    pause("Extended rectangle clipping");
}

static void test_boundary_access_safety(void) {
    gl_init(137, 89, GL_SINGLEBUFFER);
    
    gl_draw_pixel(-5, -3, GL_BLUE); // out of bounds
    gl_draw_pixel(137, 89, GL_BLUE); // out of bounds
    gl_draw_pixel(0, 0, GL_BLUE); // valid min
    gl_draw_pixel(136, 88, GL_BLUE); // valid max
    assert(gl_read_pixel(0, 0) == GL_BLUE);
    assert(gl_read_pixel(136, 88) == GL_BLUE);
    assert(gl_read_pixel(-1, -1) == 0); // out of bounds read
    assert(gl_read_pixel(137, 89) == 0); // out of bounds read
    
    gl_draw_rect(-7, -7, 150, 100, GL_RED); // partially out of bounds
    assert(gl_read_pixel(0, 0) == GL_RED);
    assert(gl_read_pixel(136, 88) == GL_RED);
    assert(gl_read_pixel(138, 90) == 0);
    
    pause("Boundary safety");
}

static void test_console_edge_cases(void) {
    console_init(25, 50, GL_WHITE, GL_BLACK);
    
    console_printf("First line\n");
    console_printf("\bSecond line"); // backspace at start
    
    console_printf("This should disappear\f"); // clear mid-text
    console_printf("This should be visible");
    
    console_printf("12345678901234567890123456789012345678901234567890MORE");
    console_printf("\nNext line should start here");
    
    pause("Console edge cases");
}

static void test_advanced_graphics_features(void) {
    gl_init(467, 389, GL_DOUBLEBUFFER);
    gl_clear(GL_BLACK);
    
    gl_draw_line(23, 67, 445, 312, GL_WHITE); // diagonal line
    gl_draw_line(23, 312, 445, 67, GL_RED); // crossed diagonal
    gl_draw_line(234, 34, 234, 355, GL_GREEN); // vertical
    gl_draw_line(45, 194, 422, 194, GL_BLUE); // horizontal
    
    gl_draw_triangle(123, 89, 345, 278, 234, 345, GL_CYAN); // normal triangle
    gl_draw_triangle(78, 56, 78, 167, 189, 111, GL_YELLOW); // right triangle
    gl_draw_triangle(345, 78, 389, 78, 367, 156, GL_MAGENTA); // isosceles
    
    gl_swap_buffer();
    pause("Lines and triangles");
    
    timer_init();
    unsigned int start = timer_get_ticks();
    
    for(int i = 0; i < 50; i++) {
        gl_draw_triangle(200, 100, 300, 300, 100, 300, GL_AMBER);
    }
    
    unsigned int end = timer_get_ticks();
    printf("Triangle rendering time: %d ms\n", (end - start) / 1000);
}

void test_console_special_chars(void) {
    console_init(10, 40, gl_color(255,255,255), gl_color(0,0,0));
    
    console_printf("abc\b\b"); // backspace
    console_printf("test\fnew"); // form feed
    char long_text[50];
    memset(long_text, 'x', 45);
    long_text[45] = '\0';
    console_printf("%s\b\b", long_text); // wrap and backspace
}

void main(void) {
    enable_fpu();  // Enable the FPU
    timer_init();
    uart_init();
    printf("Executing main() in test_gl_console.c\n");

    test_fb();
    test_gl_getters_and_colors();
    test_gl();
    test_rectangle_clipping_extended();
    test_boundary_access_safety(); //does not work; fb init issue?
    test_clipping();
    test_text_drawing();
    test_console();
    test_console_wrap_scroll();
    test_mandelbrot();
    test_console_edge_cases();
    test_advanced_graphics_features();
    test_console_special_chars();

    printf("Completed main() in test_gl_console.c\n");
}