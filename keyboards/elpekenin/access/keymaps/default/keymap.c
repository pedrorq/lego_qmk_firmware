// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>
#include "keycode.h"
#include QMK_KEYBOARD_H
#include "access.h"
#include "color.h"
#include "graphics.h"
#include "touch_driver.h"


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_4x4(
        QK_BOOT,   KC_7,   KC_8,   KC_9,
           KC_A,   KC_4,   KC_5,   KC_6,
           KC_B,   KC_1,   KC_2,   KC_3,
           KC_C, KC_SPC, KC_DOT, KC_DOT
    )
};

void keyboard_post_init_user(void) {
#if defined(QUANTUM_PAINTER_ENABLE)
    qp_drawimage(lcd, 180, 135, thermometer);
    qp_drawimage(lcd, 252, 135, message);
#endif // QUANTUM_PAINTER_ENABLE
}

#if defined (TOUCH_SCREEN)
uint32_t touch_timer = 0;
void housekeeping_task_user(void) {
    // Wait until device is initialized
    if (touch_device == NULL)
        return;

    // Read every 0.5 seconds
    if (timer_elapsed32(touch_timer) < 200)
        return;

    touch_timer = timer_read32();
    touch_report_t touch_report = get_spi_touch_report(touch_device);
#    if defined(XAP_ENABLE)
    static bool release_notified = true;
    if (touch_report.pressed) {
        uint8_t payload[4] = { touch_report.x & 0xFF, touch_report.x >> 8, touch_report.y & 0xFF, touch_report.y >> 8 };
        // 0x03 means: user-level message
        xap_broadcast(0x03, payload, sizeof(payload));

        release_notified = false;
    }

    else if (!release_notified) {
        // Send x:0, y:0 (no button there) for cleanup
        uint8_t payload[4] = { 0, 0, 0 , 0};
        xap_broadcast(0x03, payload, sizeof(payload));

        release_notified = true;
    }
#    endif // XAP_ENABLE

#    if defined(ONE_HAND_MODE)
    screen_one_hand(touch_report);
#    endif // ONE_HAND_MODE
}
#endif // TOUCH_SCREEN
