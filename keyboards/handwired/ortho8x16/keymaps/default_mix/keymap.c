/*
Copyright 2021-2022 Alin M Elena <alinm.elena@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

enum layer_names {
    _QW = 0,
    _LWR,
    _RSE,
    _ADJ,
    _TST
};

#define LOWER TT(_LWR)
#define RAISE TT(_RSE)
#define TEST TT(_TST)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_QW] = LAYOUT_mix_8x16(
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, TEST,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_TAB,  KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_CAPS, KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_LSPO, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_M, KC_UP, KC_O,
    KC_LCTL,KC_MENU, KC_LALT,KC_SPC, KC_SPC, KC_ALGR, LOWER, RAISE, KC_LEFT, KC_DOWN, KC_RGHT
),

[_LWR] = LAYOUT_mix_8x16(
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, TEST,
    QK_BOOT,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_TAB,  KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_CAPS, KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_LSPO, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_M, KC_UP, KC_O,
    KC_LCTL,KC_MENU, KC_LALT,KC_SPC, KC_SPC, KC_ALGR, LOWER, RAISE, KC_LEFT, KC_DOWN, KC_RGHT
),

[_RSE] = LAYOUT_mix_8x16(
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, TEST,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_TAB,  KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_CAPS, KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_LSPO, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_M, KC_UP, KC_O,
    KC_LCTL,KC_MENU, KC_LALT,KC_SPC, KC_SPC, KC_ALGR, LOWER, RAISE, KC_LEFT, KC_DOWN, KC_RGHT
),

[_ADJ] =  LAYOUT_mix_8x16(
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, TEST,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_TAB,  KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_CAPS, KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_LSPO, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_M, KC_UP, KC_O,
    KC_LCTL,KC_MENU, KC_LALT,KC_SPC, KC_SPC, KC_ALGR, LOWER, RAISE, KC_LEFT, KC_DOWN, KC_RGHT
),
[_TST] =  LAYOUT_mix_8x16(
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, TEST,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_A, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_TAB,  KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O,
    KC_CAPS, KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,KC_P, KC_Q,KC_R, KC_S,
    KC_LSPO, KC_B, KC_C, KC_D, KC_Z, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_M, KC_UP, KC_O,
    KC_LCTL,KC_MENU, KC_LALT,KC_SPC, KC_SPC, KC_ALGR, LOWER, RAISE, KC_LEFT, KC_DOWN, KC_RGHT
)


};
// clang-format on
static inline void led_caps(const bool on) {

   if (on) {
     led_toggle(LED_CL);
   }
}


void matrix_scan_user(void) {
    led_t led_state = host_keyboard_led_state();
    led_caps(led_state.caps_lock);
}

bool led_update_user(led_t led_state) {
    // Disable the default LED update code, so that lock LEDs could be reused to show layer status.
    return false;
}


bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case (KC_CAPS):
            led_toggle(LED_CL);
            return true;
            break;
        case (KC_NUM):
            led_toggle(LED_NL);
            return true;
            break;
        case (KC_SCRL):
            led_toggle(LED_SL);
            return true;
            break;
        case (TT(_LWR)):
            if (!record->event.pressed && record->tap.count == TAPPING_TOGGLE) {
                // This runs before the TT() handler toggles the layer state, so the current layer state is the opposite of the final one after toggle.
//                toggle_lwr = !layer_state_is(_LWR);
             led_toggle(LED_L1);
            }
            return true;
            break;
        case (TT(_RSE)):
            if (record->event.pressed && record->tap.count == TAPPING_TOGGLE) {
//                toggle_rse = !layer_state_is(_RSE);
             led_toggle(LED_L2);
            }
            return true;
            break;
        case (TT(_TST)):
            if (record->event.pressed && record->tap.count == TAPPING_TOGGLE) {
//                toggle_rse = !layer_state_is(_RSE);
             led_toggle(LED_L3);
            }
            return true;
            break;
        default:
            return true;
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _LWR, _RSE, _ADJ);
}
