/* File: keyboard.c
 * -----------------
 * ***** keyboard *****
 */
#include "keyboard.h"
#include "ps2.h"

static ps2_device_t *dev;
static keyboard_modifiers_t current_modifiers = 0;
static int caps_pressed = 0;

void keyboard_init(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    dev = ps2_new(clock_gpio, data_gpio);
}

uint8_t keyboard_read_scancode(void) {
    return ps2_read(dev);
}

key_action_t keyboard_read_sequence(void) {
    key_action_t key_event = { 0 };
    uint8_t scan_code = keyboard_read_scancode();
    uint8_t next_code = (scan_code == 0xE0) ? keyboard_read_scancode() : scan_code;
    key_event.what = (next_code == 0xF0) ? KEY_RELEASE : KEY_PRESS;
    key_event.keycode = (next_code == 0xF0) ? keyboard_read_scancode() : next_code;
    return key_event;
}

static int is_modifier(uint8_t scan_code) {
    return (scan_code >= 0x11 && scan_code <= 0x14) || scan_code == 0x58 || scan_code == 0x59;
}

static void update_modifiers(key_action_t key_event) {
    if (key_event.keycode == 0x58) {
        if (key_event.what == KEY_PRESS && !caps_pressed) {
            current_modifiers ^= KEYBOARD_MOD_CAPS_LOCK;
            caps_pressed = 1;
        } else if (key_event.what == KEY_RELEASE) {
            caps_pressed = 0;
        }
        return;
    }
    keyboard_modifiers_t flag = key_event.keycode == 0x12 || key_event.keycode == 0x59 ? 
        KEYBOARD_MOD_SHIFT : key_event.keycode == 0x14 ? KEYBOARD_MOD_CTRL : 
        key_event.keycode == 0x11 ? KEYBOARD_MOD_ALT : 0;
    current_modifiers = key_event.what == KEY_PRESS ? 
        (current_modifiers | flag) : (current_modifiers & ~flag);
}

key_event_t keyboard_read_event(void) {
    key_event_t event = { 0 };
    while (1) {
        key_action_t action = keyboard_read_sequence();
        if (is_modifier(action.keycode)) {
            update_modifiers(action);
            continue;
        }
        event.action = action;
        event.key = ps2_keys[action.keycode];
        event.modifiers = current_modifiers;
        return event;
    }
}

char keyboard_read_next(void) {
    key_event_t evt = keyboard_read_event();
    if (evt.action.what != KEY_PRESS) return keyboard_read_next();
    return ((evt.modifiers & KEYBOARD_MOD_SHIFT) || 
            ((evt.modifiers & KEYBOARD_MOD_CAPS_LOCK) && 
             (evt.key.ch & ~32) >= 'A' && (evt.key.ch & ~32) <= 'Z')) 
           ? evt.key.other_ch : evt.key.ch;
}