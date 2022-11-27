// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "access.h"

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "color.h"
#    include "graphics.h"
#    include "qp.h"
painter_device_t lcd;
#endif // QUANTUM_PAINTER_ENABLE

#if defined(ONE_HAND_ENABLE)
uint8_t one_hand_col;
uint8_t one_hand_row;
one_hand_movement_t one_hand_movement;
#endif // ONE_HAND_ENABLE

#if defined (TOUCH_SCREEN)
#    include "touch_driver.h"
touch_device_t touch_device;
#endif // TOUCH_SCREEN


void keyboard_post_init_kb(void) {
    debug_enable = true;

#if defined(DEFERRED_EXEC_ENABLE)
    // Define function so `defer_exec` doesn't crash the compiling
    uint32_t deferred_init(uint32_t trigger_time, void *cb_arg);

    // Defer init code so USB has started and we can receive console messages
    defer_exec(INIT_DELAY, deferred_init, NULL);
}

uint32_t deferred_init(uint32_t trigger_time, void *cb_arg) {
#endif // DEFERRED_EXEC_ENABLE

    dprint("---------- Init phase ----------\n");

    // =======
    // Power indicator
    setPinOutput(POWER_LED_PIN);
    writePinHigh(POWER_LED_PIN);

    // ==========
    // Peripherals
#if defined(QUANTUM_PAINTER_ENABLE)
    setPinOutput(LCD_BL_PIN);
    writePinHigh(LCD_BL_PIN);
    wait_ms(150); //Let it draw some power
    lcd = qp_ili9486_shiftreg_make_spi_device(_SCREEN_WIDTH, _SCREEN_HEIGHT, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, SPI_DIV, SPI_MODE);
    qp_init(lcd, _SCREEN_ROTATION);
    qp_rect(lcd, 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, HSV_BLACK, true);
    load_qp_resources();
    dprint("Display initialized\n");
#endif // QUANTUM_PAINTER_ENABLE

#if defined (TOUCH_SCREEN)
    static touch_driver_t touch_driver = {
        .width = _SCREEN_WIDTH,
        .height = _SCREEN_HEIGHT,
        .measurements = 1,
        .scale_x = 0.095,
        .scale_y = 0.12,
        .offset_x = -44,
        .offset_y = -23,
        .rotation = (_SCREEN_ROTATION+2)%4, // My screen is rotated 180º from touch sensor
        .upside_down = false,
        .spi_config = {
            .chip_select_pin = TOUCH_CS_PIN,
            .divisor = SPI_DIV,
            .lsb_first = false,
            .mode = SPI_MODE,
            .irq_pin = TOUCH_IRQ_PIN,
            .x_cmd = 0xD0,
            .y_cmd = 0x90
        },
    };

    touch_device = &touch_driver;
    touch_spi_init(touch_device);
    dprint("Touch initialized\n");
#endif // TOUCH_SCREEN

    dprint("\n---------- User code ----------\n");

    // =======
    // Call user code
    keyboard_post_init_user();

#if defined(DEFERRED_EXEC_ENABLE)
    return 0; //don't repeat the function
#endif // DEFERRED_EXEC_ENABLE
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    return true;
}

#if defined(ONE_HAND_ENABLE)
#    if defined (TOUCH_SCREEN)
void screen_one_hand(touch_report_t touch_report) {
    if (!touch_report.pressed) {
        return;
    }

    int16_t x = touch_report.x - SCREEN_WIDTH/2;
    int16_t y = touch_report.y - SCREEN_HEIGHT/2;

    if (x > 30) {
        one_hand_movement = y > 0 ? DIRECTION_RIGHT : DIRECTION_LEFT;
    } else if (x < -30){
        one_hand_movement = y > 0 ? DIRECTION_UP : DIRECTION_DOWN;
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    // Wait until it is initialized
    if (touch_device == NULL){
        return false;
    }

    // Get touchscreen status
    touch_report_t touch_report = get_spi_touch_report(touch_device);

    // Convert left-based to center-based coord
    int16_t x = touch_report.x - SCREEN_WIDTH/2;

    // Store previous state for comparations
    matrix_row_t previous = current_matrix[one_hand_row];

    // If screen is not pressed, neither will the key
    if (!touch_report.pressed) {
        current_matrix[one_hand_row] &= ~(1 << one_hand_col);
    }
    // If pressed and in zone, press the key
    else if (-30 < x && x < 30) {
        current_matrix[one_hand_row] |= 1 << one_hand_col;
    }

    return previous != current_matrix[one_hand_row];
}
#    endif // TOUCH_SCREEN
#endif // ONE_HAND_ENABLE
