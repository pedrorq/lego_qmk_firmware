#include QMK_KEYBOARD_H

#ifdef QUANTUM_PAINTER_ENABLE
#    include "graphics.h"
#endif // QUANTUM_PAINTER_ENABLE

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_4x4(
        QK_BOOT, KC_W, KC_A, QK_BOOT,
           KC_D, KC_W, KC_A,    KC_S,
           KC_D, KC_W, KC_A,    KC_S,
        QK_BOOT, KC_W, KC_A, QK_BOOT
    )
};

