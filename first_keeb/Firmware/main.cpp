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

ssd1306_t oled;

// will clean all these up later

bool KEY0_value = false;
bool KEY1_value = false;
bool back_btn_value = false;
bool select_btn_value = false;
bool bootsel_value = false;

bool KEY0_prev = false;
bool KEY1_prev = false;
bool back_btn_prev = false;
bool select_btn_prev = false;
bool bootsel_prev = false;

uint64_t KEY0_last_pressed_time = 0;
uint64_t KEY1_last_pressed_time = 0;
uint64_t back_last_pressed_time = 0;
uint64_t select_last_pressed_time = 0;
uint64_t bootsel_last_pressed_time = 0;

uint64_t debounce_wait_us = 5000;

bool in_menu = false;

Menu menu;

void init() {
    // init TinyUSB for HID reporting
    tusb_init();
    tud_init(BOARD_TUD_RHPORT);

    // init analog keys
    // adc_init();
    // adc_gpio_init(KEY0_GPIO);
    // adc_gpio_init(KEY1_GPIO);

    // TEMP FOR TESTING WITH REGULAR BUTTON
    gpio_init(KEY0_GPIO);
    gpio_init(KEY1_GPIO);
    gpio_pull_up(KEY0_GPIO);
    gpio_pull_up(KEY1_GPIO);

    // TEMP

    gpio_init(BACK_BTN_GPIO);
    gpio_init(SELECT_BTN_GPIO);
    gpio_pull_up(BACK_BTN_GPIO);
    gpio_pull_up(SELECT_BTN_GPIO);


    // init bootsel button
    gpio_init(BACK_BTN_GPIO);
    gpio_pull_up(BOOTSEL_GPIO);

    // init addressable LEDs
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812B_GPIO, 800000, false);
}

void read_input() {
    KEY0_prev = KEY0_value;
    KEY1_prev = KEY1_value;
    back_btn_prev = back_btn_value;
    select_btn_prev = select_btn_value;
    bootsel_prev = bootsel_value;

    

    // eager debounce - it's crude for now, will clean up later
    if (!gpio_get(KEY0_GPIO)) {
        KEY0_last_pressed_time = time_us_64();
    }

    if (!gpio_get(KEY1_GPIO)) {
        KEY1_last_pressed_time = time_us_64();
    }

    if (!gpio_get(BACK_BTN_GPIO)) {
        back_last_pressed_time = time_us_64();
    }

    if (!gpio_get(SELECT_BTN_GPIO)) {
        select_last_pressed_time = time_us_64();
    }

    if (!gpio_get(BOOTSEL_GPIO)) {
        bootsel_last_pressed_time = time_us_64();
    }

    if (time_us_64() - KEY0_last_pressed_time < debounce_wait_us) {
        KEY0_value = true;
    }
    else {
        KEY0_value = false;
    }

    if (time_us_64() - KEY1_last_pressed_time < debounce_wait_us) {
        KEY1_value = true;
    }
    else {
        KEY1_value = false;
    }

    if (time_us_64() - back_last_pressed_time < debounce_wait_us) {
        back_btn_value = true;
    }
    else {
        back_btn_value = false;
    }

    if (time_us_64() - select_last_pressed_time < debounce_wait_us) {
        select_btn_value = true;
    }
    else {
        select_btn_value = false;
    }

    if (time_us_64() - bootsel_last_pressed_time < debounce_wait_us) {
        bootsel_value = true;
    }
    else {
        bootsel_value = false;
    }
}

void send_keys() {
    // skip if HID not ready
    if (!tud_hid_ready()) {
        return;
    }

    uint8_t keycode[6] = { 0 };

    if (KEY0_value != KEY0_prev || KEY1_value != KEY1_prev) {
        if (KEY0_value) {
            keycode[0] = HID_KEY_Z;

        }
        if (KEY1_value) {
            keycode[1] = HID_KEY_X;
        }

        KEY0_prev = KEY0_value;
        KEY1_prev = KEY1_value;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
    }
}

// brightness in range 0-255. Values under 255 will lose resolution.
void put_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness = 255) {
    r *= brightness / 255;
    g *= brightness / 255;
    b *= brightness / 255;
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

        if (KEY1_value || settings.rgb_mode == RGB_STATIC) {
            put_pixel(settings.rgb_r, settings.rgb_g, settings.rgb_b);
        }
        else {
            put_pixel(0, 0, 0);
        }

        if (KEY0_value || settings.rgb_mode == RGB_STATIC) {
            put_pixel(settings.rgb_r, settings.rgb_g, settings.rgb_b);
        }
        else {
            put_pixel(0, 0, 0);
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
    }
    else {
        ssd1306_draw_string(&oled, 0, 0, 1, ("KEY0 value: " + std::to_string(KEY0_value)).c_str());
        ssd1306_draw_string(&oled, 0, 8, 1, ("KEY1 value: " + std::to_string(KEY1_value)).c_str());
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

void enter_menu() {
    menu.reset();
    while (1) {
        read_input();
        handle_leds();

        // do (value && !prev) or else its impossible to control because one click will advance in that direction many times
        // implement slow moving hold later
        if (back_btn_value && !back_btn_prev) {
            // if we cannot go back, it means we are at the root menu, so exit the menu
            int went_back_to_directory = menu.go_back();
            if (!went_back_to_directory) {
                return;
            }
        }
        else if (select_btn_value && !select_btn_prev) {
            menu.select();
        }
        else if (KEY0_value && !KEY0_prev) {
            menu.key_0_event();
        }
        else if (KEY1_value && !KEY1_prev) {
            menu.key_1_event();
        }
    }
}

void handle_bootsel() {
    // TODO have user hold bootsel button for 1 second instead of instantly executing it.
    // TODO find a way to start running new image so that rp2040 doesn't need manual reset.
    if (bootsel_prev != bootsel_value) {
        if (bootsel_value) {
            reset_usb_boot(1, 0);
        }
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
        if (select_btn_value)
        {
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
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {}
