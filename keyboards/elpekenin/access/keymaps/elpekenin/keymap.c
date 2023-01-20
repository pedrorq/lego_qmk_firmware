// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "access.h"
#include "touch_driver.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_1x1(
        QK_BOOT
    )
};

void keyboard_post_init_user() {
}

#if defined(TOUCH_SCREEN)
uint32_t touch_timer = 0;
void housekeeping_task_user(void) {
    // Wait until device is initialized
    if (ili9486_touch == NULL)
        return;

    // Read every 0.5 seconds
    if (timer_elapsed32(touch_timer) < 200)
        return;

    touch_timer = timer_read32();
#    if defined(XAP_ENABLE)
#        define PAYLOAD_SIZE 5

#        define ILI9341_ID 1
    touch_report_t ili9341_touch_report = get_spi_touch_report(ili9341_touch);
    static bool ili9341_release_notified = true;
    if (ili9341_touch_report.pressed) {
        uint8_t payload[PAYLOAD_SIZE] = { ILI9341_ID, ili9341_touch_report.x & 0xFF, ili9341_touch_report.x >> 8, ili9341_touch_report.y & 0xFF, ili9341_touch_report.y >> 8 };
        // 0x03 means: user-level message
        xap_broadcast(0x03, payload, sizeof(payload));

        ili9341_release_notified = false;
    }
    else if (!ili9341_release_notified) {
        // Send x:0, y:0 (no button there) for cleanup
        uint8_t payload[PAYLOAD_SIZE] = { ILI9341_ID };
        xap_broadcast(0x03, payload, sizeof(payload));

        ili9341_release_notified = true;
    }

#        define ILI9486_ID 2
    touch_report_t ili9486_touch_report = get_spi_touch_report(ili9486_touch);
    static bool ili9486_release_notified = true;
    if (ili9486_touch_report.pressed) {
        uint8_t payload[PAYLOAD_SIZE] = { ILI9486_ID, ili9486_touch_report.x & 0xFF, ili9486_touch_report.x >> 8, ili9486_touch_report.y & 0xFF, ili9486_touch_report.y >> 8 };
        // 0x03 means: user-level message
        xap_broadcast(0x03, payload, sizeof(payload));

        ili9486_release_notified = false;
    }
    else if (!ili9486_release_notified) {
        // Send x:0, y:0 (no button there) for cleanup
        uint8_t payload[PAYLOAD_SIZE] = { ILI9486_ID };
        xap_broadcast(0x03, payload, sizeof(payload));

        ili9486_release_notified = true;
    }
#    endif // XAP_ENABLE

#    if defined(ONE_HAND_ENABLE)
    screen_one_hand(touch_report);
#    endif // ONE_HAND_ENABLE
}
#endif // TOUCH_SCREEN
