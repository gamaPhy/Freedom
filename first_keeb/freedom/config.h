// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define ADC_RESOLUTION 12

#define DYNAMIC_KEYMAP_LAYER_COUNT 10

// corresponds to size of kb_config_t
#define EECONFIG_KB_DATA_SIZE (4 + 4 * MATRIX_ROWS * MATRIX_COLS)
#define DEBUG_MATRIX_SCAN_RATE
#define BOOTMAGIC_LITE_ROW 1
#define BOOTMAGIC_LITE_COLUMN 0

#define RGBLIGHT_EFFECT_RAINBOW_SWIRL
#define RGBLIGHT_DEFAULT_MODE (RGBLIGHT_MODE_RAINBOW_SWIRL + 5)
