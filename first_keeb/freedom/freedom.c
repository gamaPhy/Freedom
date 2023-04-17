// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#include "freedom.h"

bool calibrating_sensors = false;
kb_config_t kb_config;
const pin_t direct_pins[MATRIX_ROWS][MATRIX_COLS] = DIRECT_PINS;
const pin_scan_mode_t pin_scan_modes[MATRIX_ROWS][MATRIX_COLS] = PIN_SCAN_MODES;
uint16_t min1, max1, min2, max2, min3, max3;

void eeconfig_init_kb(void) {
    kb_config.rapid_trigger = false;
    kb_config.actuation_point_mm = 20;
    kb_config.release_point_mm = 16;
    kb_config.rapid_trigger_sensitivity_mm = 10;
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                kb_config.matrix_sensor_bounds[row][col].min = 2200;
                kb_config.matrix_sensor_bounds[row][col].max = 3000;
            }
        }
    }
    eeconfig_update_kb_datablock(&kb_config);
}

void keyboard_post_init_kb(void) {
    debug_enable = true;
    eeconfig_read_kb_datablock(&kb_config);
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
            for (int row = 0; row < MATRIX_ROWS; row++) {
                for (int col = 0; col < MATRIX_COLS; col++) {
                    if (pin_scan_modes[row][col] == ANALOG) {
                        kb_config.matrix_sensor_bounds[row][col].min = -1;
                        kb_config.matrix_sensor_bounds[row][col].max = 0;
                    }
                }
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
        if (kb_config.actuation_point_mm < 40) {
            ++kb_config.actuation_point_mm;
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    }

    return true;
}

void matrix_scan_kb(void) {
    static uint16_t key_timer;
    if (timer_elapsed(key_timer) > 1000) {
        key_timer = timer_read();
        dprintf("(%i, %i) (%i, %i) (%i, %i)\n", min1, max1, min2, max2, min3, max3);
        min1 = -1;
        max1 = 0;
        min2 = -1;
        max2 = 0;
        min3 = -1;
        max3 = 0;
    }


    if (calibrating_sensors) {
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                if (pin_scan_modes[row][col] == ANALOG) {
                    pin_t pin = direct_pins[row][col];
                    uint16_t sensor_value = 4095 - analogReadPin(pin);
                    sensor_bounds_t* bounds = &kb_config.matrix_sensor_bounds[row][col];
                    if (sensor_value < bounds->min) {
                        bounds->min = sensor_value;
                    }
                    if (sensor_value > bounds->max) {
                        bounds->max = sensor_value;
                    }
                }
            }
        }
    }
}

#ifdef VIA_ENABLE
enum via_kb_config_value {
    id_kb_rapid_trigger = 1,
    id_kb_actuation_point_mm = 2,
    id_kb_release_point_mm = 3,
    id_kb_rapid_trigger_sensitivity_mm = 4
};

void kb_config_set_value(uint8_t* data) {
    uint8_t* value_id = &(data[0]);
    uint8_t* value_data = &(data[1]);

    switch (*value_id) {
    case id_kb_rapid_trigger:
        kb_config.rapid_trigger = *value_data;
        break;
    case id_kb_actuation_point_mm:
        kb_config.actuation_point_mm = *value_data;
        break;
    case id_kb_release_point_mm:
        kb_config.release_point_mm = *value_data;
        break;
    case id_kb_rapid_trigger_sensitivity_mm:
        kb_config.rapid_trigger_sensitivity_mm = *value_data;
        break;
    }
}

void kb_config_get_value(uint8_t* data) {
    // data = [ value_id, value_data ]
    uint8_t* value_id = &(data[0]);
    uint8_t* value_data = &(data[1]);

    switch (*value_id) {
    case id_kb_rapid_trigger:
        *value_data = kb_config.rapid_trigger;
        break;
    case id_kb_actuation_point_mm:
        *value_data = kb_config.actuation_point_mm;
        break;
    case id_kb_release_point_mm:
        *value_data = kb_config.release_point_mm;
        break;
    case id_kb_rapid_trigger_sensitivity_mm:
        *value_data = kb_config.rapid_trigger_sensitivity_mm;
        break;
    }
}

void kb_config_save(void) {
    eeconfig_update_kb_datablock(&kb_config);
}

void via_custom_value_command_kb(uint8_t* data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t* command_id = &(data[0]);
    uint8_t* channel_id = &(data[1]);
    uint8_t* value_id_and_data = &(data[2]);

    if (*channel_id == id_custom_channel) {
        switch (*command_id) {
        case id_custom_set_value:
            kb_config_set_value(value_id_and_data);
            break;
        case id_custom_get_value:
            kb_config_get_value(value_id_and_data);
            break;
        case id_custom_save:
            kb_config_save();
            break;
        default:
            // Unhandled message.
            *command_id = id_unhandled;
            break;
        }
        return;
    }

    // Return the unhandled state
    *command_id = id_unhandled;
}
#endif // VIA_ENABLE
