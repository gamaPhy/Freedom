// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#include "firmware.h"

bool changing_actuation_settings = false;
bool calibrating_sensors = false;
kb_config_t kb_config;

void eeconfig_init_kb(void) {
    kb_config.rapid_trigger = true;
    kb_config.actuation_point_mm = 20;
    kb_config.release_point_mm = 16;
    for (int col = 0; col < MATRIX_COLS; col++) {
        kb_config.matrix_sensor_bounds[col].min = 0;
        kb_config.matrix_sensor_bounds[col].max = 3000;
    }
    eeconfig_update_kb_datablock(&kb_config);
}

void keyboard_post_init_kb(void) {
    eeconfig_read_kb_datablock(&kb_config);
    debug_enable = true;
    debug_matrix = true;
    debug_keyboard = true;
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    switch (keycode) {
    case KC_CALIBRATE:
        if (record->event.pressed) {
            // this will disable analog keys while calibrating
            calibrating_sensors = true;
            for (int col = 0; col < MATRIX_COLS; col++) {
                kb_config.matrix_sensor_bounds[col].min = -1;
                kb_config.matrix_sensor_bounds[col].max = 0;
            }
        } else {
            eeconfig_update_kb_datablock(&kb_config);
            calibrating_sensors = false;
        }
        return false;
    case KC_TOGGLE_RAPID_TRIGGER:
        if (record->event.pressed) {
            kb_config.rapid_trigger = !kb_config.rapid_trigger;
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    case KC_ACTUATION_DEC:
        if (kb_config.actuation_point_mm > 1) {
            --kb_config.actuation_point_mm;
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    case KC_ACTUATION_INC:
        if (kb_config.actuation_point_mm < 40){
            ++kb_config.actuation_point_mm;
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    }

    return true;
}

