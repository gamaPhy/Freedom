// Copyright 2023 Thailer lietz (@ThailerL) Theodore Lietz (@tlietz)
// SPDX-License-Identifier: GPL-3.0-or-later
#include "firmware.h"
#include "matrix.h"
#include "analog.h"
#include "print.h"

extern matrix_row_t raw_matrix[MATRIX_ROWS]; // raw values
extern matrix_row_t matrix[MATRIX_ROWS];     // debounced values
static const pin_t direct_pins[MATRIX_ROWS][MATRIX_COLS] = DIRECT_PINS;
static const int digital_row = 0;
static int previous_sensor_values[MATRIX_COLS];

int map(int input, int input_start, int input_end, int output_start, int output_end) {
    return (input - input_start) * (output_end - output_start) / (input_end - input_start) + output_start;
}

void matrix_init_custom(void) {
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            pin_t pin = direct_pins[row][col];
            if (pin != NO_PIN) {
                if (row == digital_row) {
                    setPinInputHigh(pin);
                } else {
                    palSetLineMode(pin, PAL_MODE_INPUT_ANALOG);
                }
            }
        }
    }
}

void matrix_read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {
    // Start with a clear matrix row
    matrix_row_t current_row_value = 0;

    matrix_row_t row_shifter = 1;
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
        pin_t pin = direct_pins[current_row][col_index];
        if (pin == NO_PIN) {
            continue;
        }

        if (current_row == digital_row) {
            current_row_value |= readPin(pin) ? 0 : row_shifter;
        } else {
            int sensor_value = analogReadPin(pin);
            if (calibrating_sensors) {
                sensor_bounds_t bounds = matrix_sensor_bounds[col_index];
                if (sensor_value < bounds.min) {
                    bounds.min = sensor_value;
                }
                if (sensor_value > bounds.max) {
                    bounds.max = sensor_value;
                }
            } else {
                int actuation_point_analog = map(actuation_point_mm, 0, 40, matrix_sensor_bounds[col_index].max, matrix_sensor_bounds[col_index].min);
                if (rapid_trigger_on) {
                    int previous_value = previous_sensor_values[col_index];

                    if (sensor_value == previous_value) {
                        // keep the current state
                        current_row_value |= current_matrix[current_row] & row_shifter;
                    } else {
                        current_row_value |= sensor_value > previous_value ? row_shifter : 0;
                    }

                    previous_sensor_values[col_index] = sensor_value;
                } else {
                    current_row_value |= sensor_value < actuation_point_analog ? row_shifter : 0;
                }

            }
        }
    }

    current_matrix[current_row] = current_row_value;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    matrix_row_t new_matrix[MATRIX_ROWS] = { 0 };
    for (int row = 0; row < MATRIX_ROWS; row++) {
        matrix_read_cols_on_row(new_matrix, row);
    }

    bool changed = memcmp(current_matrix, new_matrix, sizeof(new_matrix)) != 0;
    if (changed) {
        memcpy(current_matrix, new_matrix, sizeof(new_matrix));
    }
    return changed;
}
