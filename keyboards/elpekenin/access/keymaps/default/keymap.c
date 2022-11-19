#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_4x4(
        QK_BOOT, KC_W, KC_A, KC_S,
           KC_D, KC_W, KC_A, KC_S,
           KC_D, KC_W, KC_A, KC_S,
           KC_D, KC_W, KC_A, KC_S
    )
};

