// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "quantum.h"
#include "analog.h"

enum custom_keycodes {
    KC_CALIBRATE = QK_KB,
    KC_TOGGLE_RAPID_TRIGGER,
    KC_ACTUATION_DEC,
    KC_ACTUATION_INC,
    NEW_QK_KB
};

extern bool changing_actuation_settings;
extern bool calibrating_sensors;

typedef struct {
    uint16_t min;
    uint16_t max;
} sensor_bounds_t;

typedef struct {
    bool rapid_trigger;
    uint8_t actuation_point_mm;
    uint8_t release_point_mm;
    sensor_bounds_t matrix_sensor_bounds[MATRIX_COLS];
} kb_config_t;

extern kb_config_t kb_config;
