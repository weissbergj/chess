/* File: fb.c
 * ----------
 * HDMI stuff + Framebuffer
 */
#include "fb.h"
#include "de.h"
#include "hdmi.h"
#include "malloc.h"
#include "strings.h"

// Framebuffer module state
static struct {
    int pixel_width;         // Horizontal pixel count
    int pixel_height;        // Vertical pixel count
    int bytes_per_pixel;     // Bytes per pixel
    void *front_buffer;      // Front buffer memory address
    void *back_buffer;       // Back buffer memory address (for double buffering)
    fb_mode_t buffer_mode;   // Buffering mode (single/double)
} framebuffer;

void fb_init(int width, int height, fb_mode_t mode) {
    if (framebuffer.front_buffer) {
        free(framebuffer.front_buffer);
        if (framebuffer.back_buffer) free(framebuffer.back_buffer);
    }

    framebuffer.pixel_width = width;
    framebuffer.pixel_height = height;
    framebuffer.bytes_per_pixel = 4;
    framebuffer.buffer_mode = mode;
    
    int total_bytes = width * height * framebuffer.bytes_per_pixel;
    framebuffer.front_buffer = malloc(total_bytes);
    if (!framebuffer.front_buffer) return;
    memset(framebuffer.front_buffer, 0, total_bytes);
    
    if (mode == FB_DOUBLEBUFFER) {
        framebuffer.back_buffer = malloc(total_bytes);
        if (!framebuffer.back_buffer) {
            free(framebuffer.front_buffer);
            framebuffer.front_buffer = NULL;
            return;
        }
        memset(framebuffer.back_buffer, 0, total_bytes);
    } else {
        framebuffer.back_buffer = NULL;
    }

    hdmi_resolution_id_t resolution_id = hdmi_best_match(width, height);
    hdmi_init(resolution_id);
    de_init(width, height, hdmi_get_screen_width(), hdmi_get_screen_height());
    de_set_active_framebuffer(framebuffer.front_buffer);
}

int fb_get_width(void) {
    return framebuffer.pixel_width;
}

int fb_get_height(void) {
    return framebuffer.pixel_height;
}

int fb_get_depth(void) {
    return framebuffer.bytes_per_pixel;
}

void* fb_get_draw_buffer(void) {
    return framebuffer.buffer_mode == FB_DOUBLEBUFFER ? framebuffer.back_buffer : framebuffer.front_buffer;
}

void fb_swap_buffer(void) {
    if (framebuffer.buffer_mode == FB_DOUBLEBUFFER) {
        void* temp = framebuffer.front_buffer;
        framebuffer.front_buffer = framebuffer.back_buffer;
        framebuffer.back_buffer = temp;
        de_set_active_framebuffer(framebuffer.front_buffer);
    }
}