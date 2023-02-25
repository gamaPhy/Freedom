#pragma once
#include <cstdint>
#include "pico/stdlib.h"
#include "hardware/flash.h"

enum rgb_mode_t {
    RGB_KEY_DEPTH = 0,
    RGB_STATIC,
    RGB_ACTUATION,
    RGB_OFF,
    NUM_RGB_MODES
};

const std::string RGB_MODE_TEXT[] = { "Key depth",  "Static", "On actuation", "Off" };

struct Settings {
    uint8_t rapid_trigger_toggle; // 0 = off, 1 = on
    uint8_t actuation_point; // 10x the actuation point, so a value of 13 means 1.3mm
    uint8_t release_point; // 10x the release point
    uint8_t rgb_mode;
    uint8_t rgb_r;
    uint8_t rgb_g;
    uint8_t rgb_b;
};

Settings settings;

const Settings DEFAULT_SETTINGS = { 1, 10, 8, RGB_KEY_DEPTH, 40, 0, 0 };
const uint32_t SETTINGS_FLASH_OFFSET = PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;
const uint32_t SETTINGS_FLASH_ADDRESS = XIP_BASE + SETTINGS_FLASH_OFFSET;

bool core1_lockout = false;

void save_settings(Settings settings) {
    if (memcmp(&settings, (Settings*)SETTINGS_FLASH_ADDRESS, sizeof(Settings)) != 0) {
        // Need to lockout core1 during this or writing to flash crashes the program, don't know why
        core1_lockout = true;
        sleep_ms(100); // sleep to wait for core1 to stop doing stuff

        //flash_range_program only writes data in multiples of FLASH_PAGE_SIZE, so need to create this extra space
        uint8_t buf[FLASH_PAGE_SIZE];
        memcpy(buf, &settings, sizeof(Settings));
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(SETTINGS_FLASH_OFFSET, FLASH_SECTOR_SIZE);
        flash_range_program(SETTINGS_FLASH_OFFSET, buf, FLASH_PAGE_SIZE);
        restore_interrupts(ints);

        core1_lockout = false;
    }
}

Settings get_settings() {
    // have a checksum sector above saved settings in flash to check if we previously wrote settings
    const uint32_t checksum_offset = SETTINGS_FLASH_OFFSET - FLASH_SECTOR_SIZE;
    uint32_t checksum_address = uint32_t(XIP_BASE + checksum_offset);
    uint8_t checksum[FLASH_PAGE_SIZE];
    memset(checksum, 123, FLASH_PAGE_SIZE); // 123 is a random number, just need something consistent to check
    // if checksum already exists, this is not the first time booting, so get existing settings
    if (memcmp((uint8_t*)checksum_address, checksum, FLASH_PAGE_SIZE) == 0) {
        return *(Settings*)SETTINGS_FLASH_ADDRESS;
    }

    // checksum does not exist, so it is the first time booting and settings don't exist. write checksum and get deafult settings
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(checksum_offset, FLASH_SECTOR_SIZE);
    flash_range_program(checksum_offset, checksum, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    save_settings(DEFAULT_SETTINGS);

    return DEFAULT_SETTINGS;
}
