// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include <hal_pal.h>
#include "access.h"
#include "print.h"

#ifdef QUANTUM_PAINTER_ENABLE
#    include "color.h"
#    include "graphics.h"
#    include "qp.h"
painter_device_t lcd;
#endif // QUANTUM_PAINTER_ENABLE

#ifdef RGB_MATRIX_ENABLE
uint8_t key_selector_mode_last_key;
key_selector_direction_t key_selector_direction;
#endif // RGB_MATRIX_ENABLE

void keyboard_post_init_kb(void) {
    debug_enable = true;
    debug_matrix = true;

#ifdef DEFERRED_EXEC_ENABLE
    // Define function so `defer_exec` doesn't crash the compiling
    uint32_t deferred_init(uint32_t trigger_time, void *cb_arg);

    // Defer init code so USB has started and we can receive console messages
    defer_exec(2500, deferred_init, NULL);
}

uint32_t deferred_init(uint32_t trigger_time, void *cb_arg) {
    print("Running deferred code\n");
#endif // DEFERRED_EXEC_ENABLE

    // =======
    // Power indicator
    setPinOutput(POWER_LED_PIN);
    writePinHigh(POWER_LED_PIN);

    // ==========
    // Setup pins
    // -- Touch screen
    setPinOutput(TP_CS_PIN);
    writePinHigh(TP_CS_PIN); // Maybe not needed

#ifdef QUANTUM_PAINTER_ENABLE
    // -- Init display
    setPinOutput(LCD_BL_PIN);
    writePinHigh(LCD_BL_PIN);
    wait_ms(150); //Let it draw some power
    lcd = qp_ili9486_shiftreg_make_spi_device(_LCD_WIDTH, _LCD_HEIGHT, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, LCD_SPI_DIV, LCD_SPI_MODE);
    qp_init(lcd, _LCD_ROTATION);
    qp_rect(lcd, 0, 0, LCD_WIDTH-1, LCD_HEIGHT-1, HSV_BLACK, true);
    load_qp_resources();
#endif // QUANTUM_PAINTER_ENABLE

    // =======
    // Call user code
    keyboard_post_init_user();
#ifdef DEFERRED_EXEC_ENABLE
    return 0; //don't repeat the function
#endif // DEFERRED_EXEC_ENABLE
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    return true;
}

#ifdef ONE_HAND_MODE
report_mouse_t empty_mouse_report() {
    return report_mouse_t {
        .x = 0,
        .y = 0,
        .h = 0,
        .v = 0,
        .buttons = 0
    }
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (mouse_report.buttons) {
        // TODO: Change so it actually updates matrix, and stuff other than basic keycodes work
        uint8_t current_layer = get_highest_layer(layer_state);
        tap_code(keymaps[current_layer][key_selector_mode_last_key]);
        
        return empty_mouse_report();
    }

    // Update the direction
    if (abs(mouse_report.x) > abs(mouse_report.y)) {
        if (mouse_report.x > 0)
            key_selector_direction = DIRECTION_UP;
        else
            key_selector_direction = DIRECTION_LEFT;
    } else {
        if (mouse_report.y > 0)
            key_selector_direction = DIRECTION_DOWN;
        else
            key_selector_direction = DIRECTION_RIGHT;
    }
        case KC_SPC:

            return false;

        default:
            return true;

    return empty_mouse_report();
}
#endif // ONE_HAND_MODE
