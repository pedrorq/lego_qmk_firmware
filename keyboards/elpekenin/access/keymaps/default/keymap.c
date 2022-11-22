// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "keyboard.h"
#include QMK_KEYBOARD_H
#include "access.h"
#include "touch_driver.h"

#if defined(AUDIO_ENABLE)
float my_song[][2] = SONG(QWERTY_SOUND);
#endif // AUDIO_ENABLE

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_4x4(
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT,
        QK_BOOT, QK_BOOT, QK_BOOT, QK_BOOT
    )
};

#if defined (TOUCH_SCREEN)
uint32_t touch_timer = 0;
void housekeeping_task_user(void) {
    // Wait until device is initialized
    if (touch_device == NULL)
        return;

    // Read every 0.5 seconds
    if (timer_elapsed32(touch_timer) < 500)
        return;

    touch_timer = timer_read32();
    touch_report_t touch_report = get_spi_touch_report(touch_device);
    if (touch_report.pressed) {
        dprintf("Screen was pressed at x: %u, y: %u\n", touch_report.x, touch_report.y);
#if defined(ONE_HAND_MODE)
        screen_one_hand(touch_report);
#endif // ONE_HAND_MODE
#if defined(AUDIO_ENABLE)
        PLAY_SONG(my_song);
#endif // AUDIO_ENABLE
    }
}
#endif // TOUCH_SCREEN
