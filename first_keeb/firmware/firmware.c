// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#include "firmware.h"

int actuation_point_mm = 10;
bool changing_actuation_settings = false;
bool calibrating_sensors = false;
bool rapid_trigger_on = true;
sensor_bounds_t matrix_sensor_bounds[] = { {500, 1500}, {500, 1500}, {500, 1500} };

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
                matrix_sensor_bounds[col].min = (1 << ADC_RESOLUTION) - 1;
                matrix_sensor_bounds[col].max = 0;
            }
        } else {
            calibrating_sensors = false;
        }
        return false;
    case KC_TOGGLE_RAPID_TRIGGER:
        if (record->event.pressed) {
            rapid_trigger_on = !rapid_trigger_on;
        }
        return false;
    case KC_ACTUATION_DEC:
        --actuation_point_mm;
        return false;
    case KC_ACTUATION_INC:
        ++actuation_point_mm;
        return false;
    }

    return true;
}

void keyboard_post_init_user() {
    debug_enable = true;
    debug_matrix = true;
    debug_keyboard = true;
    // debug_mouse=true;
}
