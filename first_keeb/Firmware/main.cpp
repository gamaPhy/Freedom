#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <cstring>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "ws2812.pio.h"
#include "tusb_config.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "pico/multicore.h"
#include "hardware/flash.h"
extern "C" {
#include "ssd1306/ssd1306.h"
}

// analog keys
const uint8_t KEY0_GPIO = 26;
const uint8_t KEY0_ADC = 0;
const uint8_t KEY1_GPIO = 27;
const uint8_t KEY1_ADC = 1;

// addressable LEDs
const uint8_t WS2812B_GPIO = 13;

const uint8_t OLED_SDA_GPIO = 14;
const uint8_t OLED_SCL_GPIO = 15;
i2c_inst* oled_i2c = i2c1;
ssd1306_t oled;

bool KEY0_value;
bool KEY1_value;
bool KEY0_pressed = false;
bool KEY1_pressed = false;

uint32_t main_hz = 0;
uint32_t oled_hz = 0;

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
    gpio_pull_down(KEY0_GPIO);
    gpio_pull_down(KEY1_GPIO);
    // TEMP

    // init addressable LEDs
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812B_GPIO, 800000, false);
}

void read_input() {
    // adc_select_input(KEY0_ADC);
    // adc_read();
    KEY0_value = gpio_get(KEY0_GPIO);

    // adc_select_input(KEY1_ADC);
    // adc_read();
    KEY1_value = gpio_get(KEY1_GPIO);
}

void send_keys() {
    // skip if HID not ready
    if (!tud_hid_ready()) {
        return;
    }

    uint8_t keycode[6] = { 0 };

    if (KEY0_value != KEY0_pressed || KEY1_value != KEY1_pressed) {
        if (KEY0_value) {
            keycode[0] = HID_KEY_Z;

        }
        if (KEY1_value) {
            keycode[1] = HID_KEY_X;
        }

        KEY0_pressed = KEY0_value;
        KEY1_pressed = KEY1_value;

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
    static absolute_time_t timeout = make_timeout_time_ms(5);
    if (get_absolute_time() >= timeout) {
        timeout = make_timeout_time_ms(5);
        if (KEY0_value) {
            put_pixel(200, 200, 0);
        }
        else {
            put_pixel(0, 0, 0);
        }

        if (KEY1_value) {
            put_pixel(0, 200, 100);
        }
        else {
            put_pixel(0, 0, 0);
        }
    }
}

uint baud_rate;

void handle_display() {
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 0, 1, ("KEY0 value: " + std::to_string(KEY0_value)).c_str());
    ssd1306_draw_string(&oled, 0, 8, 1, ("KEY1 value: " + std::to_string(KEY1_value)).c_str());
    ssd1306_draw_string(&oled, 0, 16, 1, ("Main Hz: " + std::to_string(main_hz)).c_str());
    ssd1306_draw_string(&oled, 0, 24, 1, ("OLED Hz: " + std::to_string(oled_hz)).c_str());
    ssd1306_draw_string(&oled, 0, 32, 1, ("OLED baud Hz: " + std::to_string(baud_rate)).c_str());
    ssd1306_show(&oled);
}

void oled_control_core_entry() {
    // init OLED
    baud_rate = i2c_init(oled_i2c, 3000000);
    gpio_set_function(OLED_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA_GPIO);
    gpio_pull_up(OLED_SCL_GPIO);
    oled.external_vcc = false;
    ssd1306_init(&oled, 128, 64, 0x3C, oled_i2c);

    // reset OLED
    ssd1306_clear(&oled);
    ssd1306_show(&oled);

    absolute_time_t timeout = make_timeout_time_ms(1000);
    uint i = 0;

    while (1) {
        handle_display();
        if (get_absolute_time() > timeout) {
            timeout = make_timeout_time_ms(1000);
            oled_hz = i;
            i = 0;
        }
        ++i;
    }
}

int main() {
    init();

    multicore_launch_core1(oled_control_core_entry);

    absolute_time_t timeout = make_timeout_time_ms(1000);
    uint i = 0;

    while (1) {
        tud_task(); // you just have to do this for TinyUSB
        read_input();
        send_keys();
        handle_leds();

        if (get_absolute_time() > timeout) {
            timeout = make_timeout_time_ms(1000);
            main_hz = i;
            i = 0;
        }
        ++i;
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
