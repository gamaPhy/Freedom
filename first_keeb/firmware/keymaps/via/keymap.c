// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#include QMK_KEYBOARD_H

enum layers {
    _OSU = 0,
    _SET
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_OSU] = LAYOUT(
        KC_Z,   KC_X,   KC_C,
        KC_ESC, TO(_SET)
    ),
    [_SET] = LAYOUT(
        KC_ACTUATION_DEC, KC_TOGGLE_RAPID_TRIGGER, KC_ACTUATION_INC,
        KC_CALIBRATE, TO(_OSU)
    )
};

enum via_kb_config_value {
    id_kb_rapid_trigger = 1,
    id_kb_actuation_point_mm = 2,
    id_kb_release_point_mm = 3
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
