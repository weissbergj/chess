#include "gl.h"
#include "mouse.h"
#include "timer.h"
#include "interrupts.h"
#include "uart.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define CURSOR_SIZE 10
#define DRAW_SIZE 4

typedef struct {
    int x;
    int y;
} point_t;

#define MAX_POINTS 10000
static point_t drawn_points[MAX_POINTS];
static int num_points = 0;

static int prev_x = -1;
static int prev_y = -1;

static int abs(int x) {
    return x < 0 ? -x : x;
}

void main(void) {
    uart_init();
    interrupts_init();
    gl_init(SCREEN_WIDTH, SCREEN_HEIGHT, GL_DOUBLEBUFFER);
    mouse_init(MOUSE_CLOCK, MOUSE_DATA);
    interrupts_global_enable();
    
    gl_clear(GL_BLACK);
    gl_swap_buffer();
    
    int cur_x = SCREEN_WIDTH/2;
    int cur_y = SCREEN_HEIGHT/2;
    
    while (1) {
        mouse_event_t event = mouse_read_event();
        
        cur_x += event.dx;
        cur_y -= event.dy;
        
        if (cur_x < 0) cur_x = 0;
        if (cur_x >= SCREEN_WIDTH) cur_x = SCREEN_WIDTH-1;
        if (cur_y < 0) cur_y = 0;
        if (cur_y >= SCREEN_HEIGHT) cur_y = SCREEN_HEIGHT-1;
        
        gl_clear(GL_BLACK);
        
        if (event.left && num_points < MAX_POINTS) {
            if (prev_x != -1) {
                int dx = cur_x - prev_x;
                int dy = cur_y - prev_y;
                int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
                if (steps > 0) {
                    float x_step = (float)dx / steps;
                    float y_step = (float)dy / steps;
                    for (int i = 0; i <= steps; i++) {
                        int x = prev_x + (int)(i * x_step);
                        int y = prev_y + (int)(i * y_step);
                        drawn_points[num_points].x = x;
                        drawn_points[num_points].y = y;
                        num_points++;
                        if (num_points >= MAX_POINTS) break;
                    }
                }
            }
            prev_x = cur_x;
            prev_y = cur_y;
        } else {
            prev_x = prev_y = -1;
        }
        
        for (int i = 0; i < num_points; i++) {
            gl_draw_rect(drawn_points[i].x - DRAW_SIZE/2, 
                        drawn_points[i].y - DRAW_SIZE/2,
                        DRAW_SIZE, DRAW_SIZE, GL_RED);
        }
        
        gl_draw_rect(cur_x-5, cur_y-5, 10, 10, GL_WHITE);
        
        gl_swap_buffer();
    }
}