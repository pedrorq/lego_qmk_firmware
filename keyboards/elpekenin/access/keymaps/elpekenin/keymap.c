// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "access.h"
#include "touch_driver.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,     QK_BOOT,                 QK_BOOT,      QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT
    )
};

#if defined(TOUCH_SCREEN) && defined(INIT_EE_HANDS_RIGHT)
#        define ILI9341_ID 1
uint32_t touch_timer = 0;
void housekeeping_task_user(void) {
    // Wait until device is initialized
    if (ili9341_touch == NULL)
        return;

    // Read every 0.5 seconds
    if (timer_elapsed32(touch_timer) < 200)
        return;

    touch_timer = timer_read32();

#    if defined(XAP_ENABLE)
#        define PAYLOAD_SIZE 5
    touch_report_t ili9341_touch_report = get_spi_touch_report(ili9341_touch);
    static bool ili9341_release_notified = true;
    if (ili9341_touch_report.pressed) {
        uint8_t payload[PAYLOAD_SIZE] = { ILI9341_ID, ili9341_touch_report.x & 0xFF, ili9341_touch_report.x >> 8, ili9341_touch_report.y & 0xFF, ili9341_touch_report.y >> 8 };
        xap_broadcast(0x03, payload, sizeof(payload)); // 3 -> user-level message
        ili9341_release_notified = false;
    }
    else if (!ili9341_release_notified) {
        uint8_t payload[PAYLOAD_SIZE] = { ILI9341_ID }; // x:0, y:0 (no button there) for cleanup
        xap_broadcast(0x03, payload, sizeof(payload));
        ili9341_release_notified = true;
    }
#    endif // XAP_ENABLE

#    if defined(ONE_HAND_ENABLE)
    screen_one_hand(touch_report);
#    endif // ONE_HAND_ENABLE
}
#endif // TOUCH_SCREEN
