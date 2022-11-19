RGB_MATRIX_EFFECT(key_selector_mode)

#if defined(RGB_MATRIX_CUSTOM_EFFECT_IMPLS)

// FIXME This code assumes no unused positions on the key matrix

#    if (RGB_MATRIX_LED_COUNT != (MATRIX_COLS * MATRIX_ROWS))
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
            break;

        case DIRECTION_RIGHT:
            new_key = (key_selector_mode_last_key + 1);
            break;

        //nothing to be done
        default:
            return false;
    }

    // -- Edge cases
    if (new_key < 0) {
        // turn negative into end-of-chain index
        new_key = (MATRIX_ROWS * MATRIX_COLS) + new_key;
    }
    // turn out-of-range into start-of-chain index
    new_key %= (MATRIX_ROWS * MATRIX_COLS);

    rgb_matrix_set_color(key_selector_mode_last_key, SELECTOR_BG_COLOR);
    key_selector_mode_last_key = (uint8_t) new_key;
    key_selector_direction = DIRECTION_NONE;
    rgb_matrix_set_color(key_selector_mode_last_key, SELECTOR_SELECT_COLOR);

    return false;
}
#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
