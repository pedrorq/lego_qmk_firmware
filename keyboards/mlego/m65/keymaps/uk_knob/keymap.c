// Copyright 2020-2023 alin m elena (@alinelena, @drFaustroll)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "keymap_uk.h"
#include "alinelena.h"

// let us assume we start with both layers off
static bool toggle_lwr = false;
static bool toggle_rse = false;

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │ ⎋ │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ ⌫ │KC_MUTE
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ↹ │ q │ w │ e │ r │ t │ y │ u │ i │ o │ p │ [ │ ] │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ # │ a │ s │ d │ f │ g │ h │ j │ k │ l │ ; │ ' │ ⏎ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ⇧(│ \ │ z │ x │ c │ v │ b │ n │ m │ , │ , │ ↑ │ / │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ⎈ │mnu│lwr│⎇  │rse│   │   │   │⎇  │ ⇧)│ ← │ ↓ │ → │
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
shifted symbols
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │ ← │ ! │ " │ £ │ $ │ % │ ^ │ & │ * │ ( │ ) │ _ │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ { │ } │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ~ │ A │ S │ D │ F │ G │ H │ J │ K │ L │ : │ @ │ ⏎ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ⇧(│ | │ Z │ X │ C │ V │ B │ N │ M │ < │ > │   │ / │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │   │   │   │   │   │   │   │ ⇧)│   │   │   │
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
altgr symbols
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │ ← │ ¹ │ ² │ ³ │ € │ ½ │ ¾ │ { │ [ │ ] │ } │ \ │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ @ │ ſ │ e │ ¶ │ ŧ │ ← │ ↓ │ → │ ø │ þ │ ä │ ã │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ~ │ æ │ ß │ ð │ đ │ ŋ │ ħ │ ả │ ĸ │ ł │ á │ â │ent│
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ± │ | │ « │ » │ ¢ │ „ │ “ │ ” │ µ │ ─ │ · │   │ ạ̣̣ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │   │   │   │   │   │   │   │ ° │   │   │   │
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
altgr shift symbols
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │   │ ¡ │ ⅛ │ £ │ ¼ │ ⅜ │ ⅝ │ ⅞ │ ™ │ ± │ ° │ ¿ │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ Ω │ § │ E │ ® │ Ŧ │ ¥ │ ↑ │ ı │ Ø │ Þ │ å │ ā │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ Æ │ ẞ │ Ð │ ª │ Ŋ │ Ħ │ a │ & │ Ł │ ˝a│ ǎ │ent│
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ± │ ¦ │ ‹ │ › │ © │ ‚ │ ‘ │ ’ │ º │ × │ ÷ │   │ ȧ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │   │   │   │   │   │   │   │ ° │   │   │   │
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
 *
*/
  [_QW] = LAYOUT_ortho_5x13e(
       KC_ESC, UK_1   , UK_2    , UK_3   , UK_4    , UK_5  , UK_6  , UK_7  , UK_8   , UK_9   , UK_0   , UK_MINS, KC_BSPC , KC_MUTE,
       KC_TAB, UK_Q   , UK_W    , UK_E   , UK_R    , UK_T  , UK_Y  , UK_U  , UK_I   , UK_O   , UK_P   , UK_LBRC, UK_RBRC ,
      UK_HASH, UK_A   , UK_S    , UK_D   , UK_F    , UK_G  , UK_H  , UK_J  , UK_K   , UK_L   , UK_SCLN, UK_QUOT, KC_ENT  ,
      SC_LSPO, UK_BSLS, UK_Z    , UK_X   , UK_C    , UK_V  , UK_B  , UK_N  , UK_M   , UK_COMM, UK_DOT , KC_UP  , UK_SLSH ,
      KC_LCTL, KC_LGUI, TT(_LWR), KC_LALT, TT(_RSE), KC_SPC, KC_SPC, KC_SPC, KC_RALT, SC_RSPC, KC_LEFT, KC_DOWN, KC_RGHT),

/*
raise layer
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │ ` │MUT│Vup│Vdn│prv│ply│nxt│EXT│SLP│WAK│PSC│ ⌫ │ = │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ σ │ ω │ ε │ ρ │ ϑ │ ℝ │ τ │ ∫ │ ∞ │ π │ ⋜ │ ⋝ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ α │ ∑ │ δ │ φ │ γ │ ℏ │ ∈ │ ϰ │ λ │ 𝔼 │ ∝ │ ⏎ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ⇧(│ ≡ │ ζ │ ξ │ χ │ ν │ β │ η │ μ │ ≈ │ ± │   │ × │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │lwr│⎇  │rse│   │   │   │⎇  │ ⇧)│   │   │   │
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
raise layer shifted
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │ ¬ │   │   │   │   │   │   │   │   │   │   │ ⌫ │ + │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │MB3│ ϕ │ Ω │ ℇ │ ∇ │ θ │ ℤ │ ℂ │ ∮ │ ⊗ │ ∏ │ ≠ │ ≅ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │MB2│  ̇ │ ⨋ │ ∂ │ ψ │ Γ │ 𝓗 │ ∉ │ ∆ │ Λ │ Å │ ℒ │ ⏎ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ⇧(│ ≢ │ ∡ │ Ξ │ ⊂ │ ⊃ │ ∩ │ ∪ │ ∘ │ ≉ │ ∓ │MSU│ ⋅ │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │MB1│MB4│lwr│⎇  │rse│   │   │   │⎇  │ ⇧)│MSL│MSD│MSR│
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
MB - mouse buttons
MS - mouse directions
*/
 [_RSE] = LAYOUT_ortho_5x13e(
     KC_GRV ,    KC_MUTE,  KC_VOLU,  KC_VOLD,  KC_MPRV,  KC_MPLY,  KC_MNXT,  G(KC_P),  KC_SLEP,    KC_WAKE,    KC_PSCR,        KC_DEL,        UK_EQL, _______,
     KC_BTN3,  XP(lq,lQ),XP(lw,lW),XP(le,lE),XP(lr,lR),XP(lt,lT),XP(ly,lY),XP(lu,lU),XP(li,lI),  XP(lo,lO),  XP(lp,lP), XP(lbo1,lbo2), XP(lbc1,lbc2),
     KC_BTN2,  XP(la,lA),XP(ls,lS),XP(ld,lD),XP(lf,lF),XP(lg,lG),XP(lh,lH),XP(lj,lJ),XP(lk,lK),  XP(ll,lL),XP(ll1,lL1),   XP(lk1,lK1),       _______,
     _______,XP(ls1,ls2),XP(lz,lZ),XP(lx,lX),XP(lc,lC),XP(lv,lV),XP(lb,lB),XP(ln,lN),XP(lm,lM),XP(lc1,lC1),XP(lp1,lP1),       KC_MS_U,   XP(lq1,lQ1),
     KC_BTN1,    KC_BTN4,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    KC_MS_L,       KC_MS_D,      KC_MS_R),

/*
lower layer
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │ ⎋ │ F1│ F2│ F3│ F4│ F5│ F6│ F7│ F8│ F9│F10│F11│F12│
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ â │   │   │   │ ț │   │   │ î │   │   │   │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │ ⇪ │ ă │ ș │   │   │   │   │   │   │   │   │   │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │   │ ç │ ⊄ │ ⊅ │ ∧ │ ∨ │   │   │   │MWU│   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │   │   │   │   │   │   │   │   │MWL│MWD│MWR│
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
MW - mouse wheel
raise layer shifted
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
 * │   │   │   │   │   │   │   │   │   │   │   │   │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ Â │   │   │   │ Ț │   │   │ Î │   │   │   │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │ Ă │ Ș │   │   │   │   │   │   │   │   │   │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │   │ Ç │ ⊆ │ ⊇ │ ⊼ │ ⊻ │   │   │   │   │   │
 * ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
 * │   │   │   │   │   │   │   │   │   │   │   │   │   │
 * └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
*/
  [_LWR] = LAYOUT_ortho_5x13e(
      KC_ESC ,     KC_F1  ,   KC_F2  ,   KC_F3  ,   KC_F4  ,   KC_F5  ,  KC_F6  ,   KC_F7  ,   KC_F8  , KC_F9  , KC_F10 , KC_F11 , KC_F12  , _______,
      _______, XP(ra1,rA1),   _______,   _______,   _______, XP(rt,rT),  _______,   _______, XP(ri,rI), _______, _______, _______, _______ ,
      KC_CAPS,   XP(ra,rA), XP(rs,rS),   _______,   _______,   _______,  _______,   _______,   _______, _______, _______, _______, _______ ,
      _______,     KC_F20 ,   _______, XP(rx,rX), XP(rc,rC), XP(rv,rV), XP(rb,rB), XP(rn,rN),  _______, _______, _______, KC_WH_U, _______ ,
      _______,     _______,   _______,   _______,   _______,   _______,  _______,    _______,  _______, _______, KC_WH_L, KC_WH_D, KC_WH_R),
/*
adj layer
 * ┌───┬────┬───┬───┬───┬───┬───┬───┬───┬───┬───┬────┬────┐
 * │MOD│RMOD│   │   │   │   │   │   │   │   │   │M_T │M_SW│
 * ├───┼────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼────┼────┤
 * │HUI│HUD │M_P│   │rst│   │   │   │   │   │   │    │M_SN│
 * ├───┼────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼────┼────┤
 * │SAI│SAD │M_B│   │   │   │   │   │   │   │   │    │M_K │
 * ├───┼────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼────┼────┤
 * │VAI│VAD │M_R│   │   │   │   │   │   │   │   │    │M_X │
 * ├───┼────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼────┼────┤
 * │TOG│    │   │   │   │   │   │   │   │   │   │M_TW│M_G │
 * └───┴────┴───┴───┴───┴───┴───┴───┴───┴───┴───┴────┴────┘
*/
  [_ADJ] = LAYOUT_ortho_5x13e(
      RGB_MOD, RGB_RMOD, A(KC_F2), _______, _______, _______, _______, _______, _______, _______, _______, RGB_M_T , RGB_M_SW, _______,
      RGB_HUI, RGB_HUD , RGB_M_P , _______, QK_BOOT, _______, _______, _______, _______, _______, _______, _______ , RGB_M_SN,
      RGB_SAI, RGB_SAD , RGB_M_B , _______, UC_NEXT, _______, _______, _______, _______, _______, _______, _______ , RGB_M_K ,
      RGB_VAI, RGB_VAD , RGB_M_R , _______, UC_PREV, UC_LINX,  UC_WIN, UC_WINC,  UC_MAC, UC_EMAC, _______, _______ , RGB_M_X ,
      RGB_TOG, _______ , _______ , _______, _______, _______, _______, _______, _______, _______, _______, RGB_M_TW, RGB_M_G),
};
// clang-format on

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [_QW] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
#    ifdef RGBLIGHT_ENABLE
    [_LWR] = {ENCODER_CCW_CW(RGB_HUD, RGB_HUI)},
    [_RSE] = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [_ADJ] = {ENCODER_CCW_CW(RGB_RMOD, RGB_MOD)},
#    else
    [_LWR] = {ENCODER_CCW_CW(KC_MNXT, KC_MPRV)},
    [_RSE] = {ENCODER_CCW_CW(KC_MFFD, KC_MRWD)},
    [_ADJ] = {ENCODER_CCW_CW(KC_PGDN, KC_PGUP)},
#    endif
};
#endif

void matrix_scan_user(void) {
    toggle_leds(toggle_lwr, toggle_rse);
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, col: %2u, row: %2u, pressed: %u, time: %5u, int: %u, count: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif

    switch (keycode) {
        case (TT(_LWR)):
            if (!record->event.pressed && record->tap.count == TAPPING_TOGGLE) {
                // This runs before the TT() handler toggles the layer state, so the current layer state is the opposite of the final one after toggle.
                set_led_toggle(_LWR, !layer_state_is(_LWR));
            }
            return true;
            break;
        case (TT(_RSE)):
            if (record->event.pressed && record->tap.count == TAPPING_TOGGLE) {
                set_led_toggle(_RSE, !layer_state_is(_RSE));
            }
            return true;
            break;
        default:
            return true;
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
#ifdef RGBLIGHT_ENABLE

    set_rgb_layers(state);

#endif

    return update_tri_layer_state(state, _LWR, _RSE, _ADJ);
}

void set_led_toggle(const uint8_t layer, const bool state) {
    switch (layer) {
        case _LWR:
            toggle_lwr = state;
            break;
        case _RSE:
            toggle_rse = state;
            break;
        default:
            break;
    }
}

void keyboard_post_init_user(void) {
#ifdef RGBLIGHT_ENABLE
    init_rgb_layers();
#endif

    init_lwr_rse_led();

#ifdef OLED_ENABLE
    init_timer();
#endif
  debug_enable=true;
  debug_matrix=true;

}
