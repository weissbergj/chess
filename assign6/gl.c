/* File: gl.c
 * ----------
 * Graphics
 */
#include "gl.h"
#include "fb.h"
#include "font.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

static float my_sqrtf(float x) {
    if (x <= 0) return 0;
    float estimate = x / 2.0f;
    for (int i = 0; i < 10; i++) {
        estimate = (estimate + x / estimate) / 2.0f;
    }
    return estimate;
}

static color_t* framebuffer_pixels;

void gl_init(int width, int height, gl_mode_t mode) {
    fb_init(width, height, mode);
    framebuffer_pixels = (color_t*)fb_get_draw_buffer();
}

int gl_get_width(void) {
    return fb_get_width();
}

int gl_get_height(void) {
    return fb_get_height();
}

color_t gl_color(uint8_t r, uint8_t g, uint8_t b) {
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void gl_clear(color_t color) {
    int total_pixels = gl_get_width() * gl_get_height();
    for (int i = 0; i < total_pixels; i++) {
        framebuffer_pixels[i] = color;
    }
}

void gl_swap_buffer(void) {
    fb_swap_buffer();
    framebuffer_pixels = (color_t*)fb_get_draw_buffer();
}

void gl_draw_pixel(int x, int y, color_t color) {
    if (x >= 0 && x < gl_get_width() && y >= 0 && y < gl_get_height()) {
        framebuffer_pixels[y * gl_get_width() + x] = color;
    }
}

color_t gl_read_pixel(int x, int y) {
    if (x >= 0 && x < gl_get_width() && y >= 0 && y < gl_get_height()) {
        return framebuffer_pixels[y * gl_get_width() + x];
    }
    return 0;
}

void gl_draw_rect(int x, int y, int width, int height, color_t color) {
    if (!framebuffer_pixels || width <= 0 || height <= 0) return;
    int x_start = MAX(0, x), y_start = MAX(0, y);
    int x_end = MIN(gl_get_width(), x + width), y_end = MIN(gl_get_height(), y + height);
    int stride = gl_get_width();
    for (int current_y = y_start; current_y < y_end; current_y++) {
        color_t* row = &framebuffer_pixels[current_y * stride];
        for (int current_x = x_start; current_x < x_end; current_x++) {
            row[current_x] = color;
        }
    }
}

static bool font_pixel_set(const uint8_t buffer[], int x, int y) {
    return buffer[y * font_get_glyph_width() + x] == 0xFF;
}

void gl_draw_char(int x, int y, char ch, color_t color) {
    unsigned char buffer[font_get_glyph_size()];
    if (!font_get_glyph(ch,buffer, sizeof(buffer))) return;
    int char_width = gl_get_char_width(), char_height = gl_get_char_height();
    for (int dy = 0; dy < char_height; dy++) {
        for (int dx = 0; dx < char_width; dx++) {
            if (font_pixel_set(buffer, dx, dy)) {
                gl_draw_pixel(x + dx, y + dy, color);
            }
        }
    }
}

void gl_draw_string(int x, int y, const char* str, color_t color) {
    int char_width = gl_get_char_width(), screen_width = gl_get_width();
    while (*str && x < screen_width) {
        gl_draw_char(x, y, *str++,color);
        x += char_width;
    }
}

int gl_get_char_height(void) {
    return font_get_glyph_height();
}

int gl_get_char_width(void) {
    return font_get_glyph_width();
}

static color_t blend_colors(color_t color1, color_t color2, float alpha) {
    alpha = alpha < 0 ? 0 : (alpha > 1 ? 1 : alpha);
    uint8_t r1 = (color1 >> 16) & 0xFF, g1 = (color1 >> 8) & 0xFF, b1 = color1 & 0xFF;
    uint8_t r2 = (color2 >> 16) & 0xFF, g2 = (color2 >> 8) & 0xFF, b2 = color2 & 0xFF;
    uint8_t r = (uint8_t)(r1 * alpha + r2* (1.0f -alpha) + 0.5f);
    uint8_t g = (uint8_t)(g1 * alpha +g2 * (1.0f - alpha) + 0.5f);
    uint8_t b = (uint8_t)(b1 * alpha + b2 * (1.0f - alpha) + 0.5f);
    return gl_color(r, g, b);
}

void gl_draw_line(int x1, int y1, int x2, int y2, color_t color) {
    if ((x1 < 0 && x2 < 0) || (y1 < 0 && y2 < 0) ||
        (x1 >= gl_get_width() && x2 >=gl_get_width()) ||
        (y1 >= gl_get_height() && y2 >= gl_get_height())) return;
    float dx = x2 - x1, dy = y2 - y1, length = my_sqrtf(dx * dx + dy * dy);
    if (length < 1) {
        if (x1 >= 0 && x1 < gl_get_width() && y1 >= 0 && y1 < gl_get_height()) {
            gl_draw_pixel(x1, y1, color);
        }
        return;
    }
    dx /= length; dy /= length;
    float x = x1, y = y1;
    for (float i = 0; i <= length; i++) {
        int ix = (int)x, iy = (int)y;
        float fx = x - ix, fy = y - iy;
        if (ix >= 0 && ix <gl_get_width() && iy >= 0 && iy < gl_get_height()) {
            color_t current = gl_read_pixel(ix, iy);
            gl_draw_pixel(ix, iy, blend_colors(color, current, (1-fx)*(1-fy)));
            if (ix + 1 < gl_get_width()) {
                current = gl_read_pixel(ix+1, iy);
                gl_draw_pixel(ix+1, iy, blend_colors(color,current, fx*(1-fy)));
            }
            if (iy + 1 < gl_get_height()) {
                current = gl_read_pixel(ix, iy+1);
                gl_draw_pixel(ix, iy+1, blend_colors(color, current, (1-fx)*fy));
            }
        }
        x += dx; y += dy;
    }
}

void gl_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, color_t color) {
    gl_draw_line(x1, y1, x2, y2, color);
    gl_draw_line(x2, y2, x3, y3, color);
    gl_draw_line(x3, y3, x1, y1, color);
    int minY = MIN(y1, MIN(y2, y3)), maxY = MAX(y1, MAX(y2, y3));
    for (int y = minY; y <= maxY; y++) {
        float x_intersect1 = -1,x_intersect2 = -1;
        if (y1 != y2 && y >= MIN(y1,y2) && y <= MAX(y1,y2)) {
            x_intersect1 = x1 + (x2-x1) * (y-y1) / (float)(y2-y1);
        }
        if (y2 != y3 && y >= MIN(y2,y3) && y <= MAX(y2,y3)) {
            float x = x2 + (x3-x2) * (y-y2) /(float)(y3-y2);
            if (x_intersect1 < 0) x_intersect1 = x; else x_intersect2 = x;
        }
        if (y3 != y1 && y >= MIN(y3,y1) && y <= MAX(y3,y1)) {
            float x = x3 + (x1-x3) * (y-y3) / (float)(y1-y3);
            if (x_intersect1 < 0) x_intersect1 = x; else x_intersect2 = x;
        }
        if (x_intersect1 >= 0 && x_intersect2 >= 0) {
            int start = MIN((int)x_intersect1, (int)x_intersect2), end =MAX((int)x_intersect1, (int)x_intersect2);
            for (int x = start; x <= end; x++) {
                gl_draw_pixel(x, y, color);
            }
        }
    }
}