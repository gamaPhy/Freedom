#pragma once
#include <cstdint>
#include "hardware/gpio.h"

struct Input {
    Input(uint8_t gpio, uint8_t hid_key): gpio(gpio), hid_key(hid_key) { }
    bool pressed;
    bool prev_pressed;
    uint64_t last_time_pressed;
    uint8_t gpio;
    uint8_t hid_key;
};

const uint8_t NUM_KEYS = 2;
enum input_t { KEY0, KEY1, BACK_BTN, SELECT_BTN, BOOTSEL_BTN };
std::vector<Input> inputs = { {26, HID_KEY_Z}, {27, HID_KEY_X}, {16, HID_KEY_NONE}, {17, HID_KEY_NONE}, {0, HID_KEY_NONE} };

uint64_t debounce_wait_us = 5000;

// addressable LEDs
const uint8_t WS2812B_GPIO = 13;

const uint8_t OLED_SDA_GPIO = 14;
const uint8_t OLED_SCL_GPIO = 15;
i2c_inst* oled_i2c = i2c1;
ssd1306_t oled;
