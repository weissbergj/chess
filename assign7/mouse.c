#include "mouse.h"
#include "ps2.h"
#include "uart.h"

#define PS2_MOUSE_CMD_RESET 0xFF
#define PS2_MOUSE_CMD_ENABLE_DATA 0xF4
#define PS2_MOUSE_RESPONSE_ACK 0xFA

static ps2_device_t *mouse_dev;

void mouse_init(gpio_id_t clock, gpio_id_t data) {
    mouse_dev = ps2_new(clock, data);
    ps2_write(mouse_dev, PS2_MOUSE_CMD_RESET);
    uint8_t ack = ps2_read(mouse_dev);
    uint8_t self_test = ps2_read(mouse_dev);
    uint8_t device_id = ps2_read(mouse_dev);
    ps2_write(mouse_dev, PS2_MOUSE_CMD_ENABLE_DATA);
    ack = ps2_read(mouse_dev);
}

mouse_event_t mouse_read_event(void) {
    mouse_event_t event = {0};
    uint8_t status = ps2_read(mouse_dev);
    uint8_t dx = ps2_read(mouse_dev);
    uint8_t dy = ps2_read(mouse_dev);
    event.left = (status & 0x01);
    event.dx = dx;
    event.dy = dy;
    return event;
}