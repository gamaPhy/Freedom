// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#include QMK_KEYBOARD_H

enum my_keykodes {
    CU_CYCLE = SAFE_RANGE,
    CU_SETTINGS
};

enum layer_t {
    _OSU = 0,
    _WORK
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_OSU] = LAYOUT(
        KC_Z, KC_X,
        CU_CYCLE, CU_SETTINGS
    )
};
