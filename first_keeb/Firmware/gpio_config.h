#pragma once
#include <cstdint>
#include "hardware/gpio.h"

// analog keys
const uint8_t KEY0_GPIO = 26;
const uint8_t KEY0_ADC = 0;
const uint8_t KEY1_GPIO = 27;
const uint8_t KEY1_ADC = 1;

const uint8_t BACK_BTN_GPIO = 16;
const uint8_t SELECT_BTN_GPIO = 17;

// addressable LEDs
const uint8_t WS2812B_GPIO = 13;

const uint8_t OLED_SDA_GPIO = 14;
const uint8_t OLED_SCL_GPIO = 15;
i2c_inst* oled_i2c = i2c1;
