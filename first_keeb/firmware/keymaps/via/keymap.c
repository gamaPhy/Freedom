// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#include QMK_KEYBOARD_H

enum layers {
    _OSU = 0,
    _SET
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_OSU] = LAYOUT(
        KC_CALIBRATE, TO(_SET),
        KC_Z,   KC_X,   KC_C
    ),
    [_SET] = LAYOUT(
        XXXXXXX, TO(_OSU),
        KC_ACTUATION_DEC, KC_TOGGLE_RAPID_TRIGGER, KC_ACTUATION_INC
    )
};
