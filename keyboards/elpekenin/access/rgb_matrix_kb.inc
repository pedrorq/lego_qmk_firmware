// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

RGB_MATRIX_EFFECT(key_selector_mode)

#if defined(RGB_MATRIX_CUSTOM_EFFECT_IMPLS)

// TODO This code assumes no unused positions on the key matrix... and a few other things

#    define MATRIX_KEYS (MATRIX_COLS*MATRIX_ROWS)

#    if (RGB_MATRIX_LED_COUNT != MATRIX_KEYS)
#        error "This effect doesn't make sense if LEDs != keys"
#    endif

#    include "access.h"
#    define SELECTOR_BG_COLOR     0, 0, 0
#    define SELECTOR_SELECT_COLOR 0, 0, RGB_MATRIX_MAXIMUM_BRIGHTNESS


static void key_selector_mode_init(effect_params_t *param) {
    key_selector_mode_last_key = 0;
    key_selector_direction = DIRECTION_NONE;
    rgb_matrix_set_color_all(SELECTOR_BG_COLOR);
    rgb_matrix_set_color(key_selector_mode_last_key, SELECTOR_SELECT_COLOR);
}

static bool key_selector_mode(effect_params_t *params) {
    if (params->init) {
        key_selector_mode_init(params);
    }

    int8_t new_key;

    switch (key_selector_direction) {
        //set new key's color accordingly
        case DIRECTION_UP:
            new_key = (key_selector_mode_last_key - MATRIX_COLS);
            break;

        case DIRECTION_DOWN:
            new_key = (key_selector_mode_last_key + MATRIX_COLS);
            break;

        case DIRECTION_LEFT:
            new_key = (key_selector_mode_last_key - 1);

            // From 1st column to last
            if ((key_selector_mode_last_key%MATRIX_COLS) == 0) {
                new_key += MATRIX_COLS;
            }
            break;

        case DIRECTION_RIGHT:
            new_key = (key_selector_mode_last_key + 1);

            // From last column to 1st
            if (((key_selector_mode_last_key+1)%MATRIX_COLS) == 0) {
                new_key -= MATRIX_COLS;
            }
            break;

        case DIRECTION_NONE:
            return false;
    }

    // Out of bounds
    if (new_key < 0) {
        new_key += MATRIX_KEYS;
    } else if (new_key >= MATRIX_KEYS) {
        new_key -= MATRIX_KEYS;
    }

    rgb_matrix_set_color(key_selector_mode_last_key, SELECTOR_BG_COLOR);
    key_selector_mode_last_key = (uint8_t) new_key;
    key_selector_direction = DIRECTION_NONE;
    rgb_matrix_set_color(key_selector_mode_last_key, SELECTOR_SELECT_COLOR);

    return false;
}
#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
