// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define RGB_DI_PIN GP13
#define RGB_MATRIX_LED_COUNT 2

#define PIN_SCAN_MODES { \
    { ANALOG,  ANALOG,  ANALOG }, \
    { DIGITAL, DIGITAL, NO_SCAN } \
}
