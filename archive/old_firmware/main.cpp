#define _GNU_SOURCE
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <cstring>
#include <variant>
#include <vector>
#include <algorithm>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "ws2812.pio.h"
#include "tusb_config.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/flash.h"
#include <hardware/sync.h>
extern "C" {
#include "ssd1306/ssd1306.h"
}

#include "settings_util.h"
#include "menu.h"
#include "gpio_config.h"

Menu menu;
bool in_menu = false;

void init() {
    // init TinyUSB for HID reporting
    tusb_init();
    tud_init(BOARD_TUD_RHPORT);

    for (Input& input : inputs) {
        gpio_init(input.gpio);
        gpio_pull_up(input.gpio);
    }

    // init addressable LEDs
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812B_GPIO, 800000, false);
}

void read_input() {
    for (Input& input : inputs) {
        input.prev_pressed = input.pressed;
    }

    // eager debounce
    for (Input& input : inputs) {
        if (!gpio_get(input.gpio)) {
            input.last_time_pressed = time_us_64();
        }

        input.pressed = time_us_64() - input.last_time_pressed < debounce_wait_us;
    }
}

void send_keys() {
    // skip if HID not ready
    if (!tud_hid_ready()) {
        return;
    }

    static uint8_t keycode_prev[6] = { 0 };
    uint8_t keycode[6] = { 0 };

    for (int key_num = 0; key_num < NUM_KEYS; ++key_num) {
        if (inputs[key_num].pressed) {
            keycode[key_num] = inputs[key_num].hid_key;
        }
    }

    if (memcmp(keycode_prev, keycode, 6) != 0) {
        memcpy(keycode_prev, keycode, 6);
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
    }
}

// brightness in range 0-255. Values under 255 will lose resolution.
void put_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness = 255) {
    r = r * brightness / 255;
    g = g * brightness / 255;
    b = b * brightness / 255;
    pio_sm_put_blocking(pio0, 0, (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b)) << 8u);
}

void handle_leds() {
    if (settings.rgb_mode == RGB_OFF) {
        return;
    }
    // send new data to LEDs every 500 microseconds
    static uint64_t last_message_time = time_us_64();
    if (time_us_64() > last_message_time + 500) {
        last_message_time = time_us_64();

        for (int key_num = 0; key_num <= NUM_KEYS; ++key_num) {
            if (inputs[key_num].pressed || settings.rgb_mode == RGB_STATIC) {
                put_pixel(settings.rgb_r, settings.rgb_g, settings.rgb_b);
            } else {
                put_pixel(0, 0, 0);
            }
        }
    }
}

void handle_display() {
    ssd1306_clear(&oled);

    if (in_menu) {
        uint32_t y = 0;
        for (std::string line : menu.get_display_strings()) {
            ssd1306_draw_string(&oled, 0, y, 1, line.c_str());
            y += 8;
        }
    } else {
        ssd1306_draw_string(&oled, 0, 0, 1, ("KEY0 press: " + std::to_string(inputs[KEY0].pressed)).c_str());
        ssd1306_draw_string(&oled, 0, 8, 1, ("KEY1 press: " + std::to_string(inputs[KEY1].pressed)).c_str());
    }

    ssd1306_show(&oled);
}

void oled_control_core_entry() {
    // init OLED
    i2c_init(oled_i2c, 3000000);
    gpio_set_function(OLED_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA_GPIO);
    gpio_pull_up(OLED_SCL_GPIO);
    oled.external_vcc = false;
    ssd1306_init(&oled, 128, 64, 0x3C, oled_i2c);

    // reset OLED
    ssd1306_clear(&oled);
    ssd1306_show(&oled);

    while (1) {
        while (core1_lockout) {
            sleep_ms(5); // idk why I need this but it freezes without it 
        }
        handle_display();
    }
}

bool newly_pressed(input_t input) { return inputs[input].pressed && !inputs[input].prev_pressed; }

void enter_menu() {
    menu.reset();
    while (1) {
        read_input();
        handle_leds();

        // implement slow moving hold later
        if (newly_pressed(BACK_BTN)) {
            // if we cannot go back, it means we are at the root menu, so exit the menu
            int went_back_to_directory = menu.go_back();
            if (!went_back_to_directory) {
                return;
            }
        } else if (newly_pressed(SELECT_BTN)) {
            menu.select();
        } else if (newly_pressed(KEY0)) {
            menu.key_0_event();
        } else if (newly_pressed(KEY1)) {
            menu.key_1_event();
        }
    }
}

void handle_bootsel() {
    // TODO have user hold bootsel button for 1 second instead of instantly executing it.
    if (inputs[BOOTSEL_BTN].pressed) {
        // GPIO 1 will be HIGH when in bootsel mode
        reset_usb_boot(1, 0);
    }
}

int main() {
    init();
    settings = get_settings();
    multicore_launch_core1(oled_control_core_entry);

    while (1) {
        tud_task(); // have to do this for TinyUSB
        read_input();
        send_keys();
        handle_leds();
        handle_bootsel();
        if (inputs[SELECT_BTN].pressed) {
            in_menu = true;
            enter_menu();
            in_menu = false;
        }
    }
}

// Not using the following callbacks:

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) { return 0; }

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) { }
