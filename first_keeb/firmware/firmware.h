// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "quantum.h"
#include "analog.h"

enum custom_keycodes {
    KC_SET_ACTUATION = QK_USER,
    KC_CALIBRATE,
    KC_TOGGLE_RAPID_TRIGGER,
    KC_ACTUATION_DEC,
    KC_ACTUATION_INC,
    NEW_QK_USER
};

// 10 = 1.0mm
extern int actuation_point_mm;
extern bool changing_actuation_settings;
extern bool calibrating_sensors;
extern bool rapid_trigger_on;

typedef struct {
    int min;
    int max;
} sensor_bounds_t;

extern sensor_bounds_t  matrix_sensor_bounds[MATRIX_COLS];
