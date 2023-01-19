// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "access.h"
#include "color.h"
#include "custom_spi_master.h"
#include "graphics.h"
#include "qp.h"
#include "sipo_pins.h"
#include "version.h"

#if defined(CUSTOM_EEPROM_ENABLE)
#    include "custom_eeprom.h"
#endif // CUSTOM_EEPROM_ENABLE

#if defined(ONE_HAND_ENABLE)
uint8_t one_hand_col;
uint8_t one_hand_row;
one_hand_movement_t one_hand_movement;
#endif // ONE_HAND_ENABLE

// eInk is on left side, dont allocate framebuffer on right
#if defined(INIT_EE_HANDS_LEFT)
#    include "qp_eink_panel.h"
painter_device_t il91874;
uint8_t il91874_buffer[EINK_3C_BYTES_REQD(IL91874_WIDTH, IL91874_HEIGHT)];

uint32_t flush_display(uint32_t trigger_time, void *device) {
    qp_flush((painter_device_t *)device);
    return 0;
}
#else
painter_device_t ili9163;
painter_device_t ili9341;
#endif // INIT_EE_HANDS_LEFT
//painter_device_t ssd1680;
//uint8_t il91874_buffer[EINK_3C_BYTES_REQD(IL91874_WIDTH, IL91874_HEIGHT)];
//uint8_t ssd1680_buffer[EINK_3C_BYTES_REQD(SSD1680_WIDTH, SSD1680_HEIGHT)];


#if defined (TOUCH_SCREEN_ENABLE)
#    include "touch_driver.h"
touch_device_t ili9341_touch;
#endif // TOUCH_SCREEN_ENABLE

// Version info
char build_date[] = QMK_BUILDDATE;
char commit_hash[] = QMK_GIT_HASH;

uint32_t deferred_init(uint32_t trigger_time, void *cb_arg) {
    dprint("---------- Init phase ----------\n");

    // =======
    // Power indicator
    setPinOutput(POWER_LED_PIN);
    writePinHigh(POWER_LED_PIN);

    // =======
    // QP
    load_qp_resources();
    wait_ms(150); //Let screens draw some power

#if defined(INIT_EE_HANDS_LEFT)
    configure_sipo_pins(
        __PADDING__,
        IL91874_RST_PIN,
        IL91874_SRAM_CS_PIN,
        SCREENS_DC_PIN,
        IL91874_CS_PIN
    );

    il91874 = qp_il91874_no_sram_make_spi_device(_IL91874_WIDTH, _IL91874_HEIGHT, IL91874_CS_PIN, SCREENS_DC_PIN, IL91874_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE, true, (void *)il91874_buffer);
    load_display(il91874);
    qp_init(il91874, IL91874_ROTATION);
    // ----- Init screens
    // il91874 = qp_il91874_no_ram_make_spi_device(_IL91874_WIDTH, _IL91874_HEIGHT, TESTS_CS_PIN, TESTS_DC_PIN, TESTS_RST_PIN, SPI_DIV, SPI_MODE, (void *)il91874_buffer, true);
 //   il91874 = qp_il91874_with_ram_make_spi_device(_IL91874_WIDTH, _IL91874_HEIGHT, TESTS_CS_PIN, TESTS_DC_PIN, TESTS_RST_PIN, SPI_DIV, SPI_MODE, (void *)il91874_buffer, true, TESTS_RAM_CS_PIN);
 //   load_display(il91874);
 //   qp_init(il91874, IL91874_ROTATION);

    // draw on it after timeout, preventing damage if replug fast
    eink_panel_dc_reset_with_sram_painter_device_t *eink = (eink_panel_dc_reset_with_sram_painter_device_t *)il91874;
    defer_exec(eink->eink_base.timeout, flush_display, (void *)eink);

    // show EEPROM state (enable features)
#    if defined(CUSTOM_EEPROM_ENABLE)
    // enabled features based on `#define`s on current firmware
    uint32_t current_config = custom_eeprom_generate();

    // if any change has ocurred, we could run some logic
    if (current_config != eeconfig_read_user()) {
        dprintf("EEPROM config has changed\n");
        eeconfig_update_user(current_config);
    }

    custom_eeprom_draw_config((void *)il91874);
#    endif // CUSTOM_EEPROM_ENABLE

    // show commit's hash
    painter_font_handle_t font       = qp_fonts[0];
    int16_t               hash_width = qp_textwidth(font, commit_hash);
    qp_drawtext_recolor(il91874, IL91874_WIDTH-hash_width, IL91874_HEIGHT-font->line_height, font, commit_hash, HSV_BLACK, HSV_WHITE);
#else // --------------------  Right half --------------------
    configure_sipo_pins(
        ILI9341_TOUCH_CS_PIN,
        SCREENS_DC_PIN,
        ILI9163_RST_PIN,
        ILI9163_CS_PIN,
        ILI9341_RST_PIN,
        ILI9341_CS_PIN
    );

    ili9163 = qp_ili9163_make_spi_device(_ILI9163_WIDTH, _ILI9163_HEIGHT, ILI9163_CS_PIN, SCREENS_DC_PIN, ILI9163_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
    load_display(ili9163);
    qp_init(ili9163, ILI9163_ROTATION);

    ili9341 = qp_ili9341_make_spi_device(_ILI9341_WIDTH, _ILI9341_HEIGHT, ILI9341_CS_PIN, SCREENS_DC_PIN, ILI9341_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
    load_display(ili9341);
    qp_init(ili9341, ILI9341_ROTATION);

    qp_rect(ili9163, 0, 0, ILI9163_WIDTH, ILI9163_HEIGHT, HSV_BLACK, true);
    qp_rect(ili9341, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, HSV_BLACK, true);
#endif
    dprint("Quantum painter devices initialised\n");

    ssd1680 = qp_ssd1680_bw_make_spi_device(_SSD1680_WIDTH, _SSD1680_HEIGHT, TESTS_CS_PIN, TESTS_DC_PIN, TESTS_RST_PIN, SPI_DIV, SPI_MODE, (void *)ssd1680_buffer,false);
    load_display(ssd1680);
    qp_init(ssd1680, SSD1680_ROTATION);

    // ----- Fill them black
    // qp_rect(ili9163, 0, 0, ILI9163_WIDTH, ILI9163_HEIGHT, HSV_BLACK, true);

    // qp_rect(ili9341, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, HSV_BLACK, true);

    // qp_rect(ili9486, 0, 0, ILI9486_WIDTH, ILI9486_HEIGHT, HSV_BLACK, true);
    // qp_drawimage(ili9486, 0, 0, qp_images[2]);

    qp_rect(ssd1680, 0, 0, SSD1680_WIDTH, SSD1680_HEIGHT, HSV_BLACK, true);
 //   qp_drawimage_recolor(ssd1680, 50, 70, qp_images[0], HSV_BLACK, HSV_RED);
    qp_flush(ssd1680);

    dprint("Quantum painter ready\n");
#endif // QUANTUM_PAINTER_ENABLE

#if defined (TOUCH_SCREEN_ENABLE)
#    error "touch screen code isn't adjusted for SIPO yet"
    // Calibration isn't very precise
    static touch_driver_t ili9341_touch_driver = {
        .width = _ILI9341_WIDTH,
        .height = _ILI9341_HEIGHT,
        .measurements = 1,
        .scale_x = 0.07,
        .scale_y = -0.09,
        .offset_x = -26,
        .offset_y = 345,
        .rotation = ILI9341_ROTATION,
        .upside_down = false,
        .spi_config = {
            .chip_select_pin = ILI9341_TOUCH_CS_PIN,
            .divisor = SCREENS_SPI_DIV,
            .lsb_first = false,
            .mode = SCREENS_SPI_MODE,
            .irq_pin = ILI9341_TOUCH_IRQ_PIN,
            .x_cmd = 0xD0,
            .y_cmd = 0x90
        },
    };
    ili9341_touch = &ili9341_touch_driver;
    touch_spi_init(ili9341_touch);

    dprint("Touch devices initialised\n");
#endif // TOUCH_SCREEN_ENABLE

    dprint("\n---------- User code ----------\n");

    // =======
    // Call user code
    keyboard_post_init_user();

    return 0;
}

void keyboard_post_init_kb(void) {
    debug_enable = true;
    debug_matrix = true;

    defer_exec(INIT_DELAY, deferred_init, NULL);
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    return true;
}

void matrix_init_custom(void) {
    setPinOutput(PISO_CS_PIN);
    writePinHigh(PISO_CS_PIN);
    custom_spi_init(REGISTERS_SPI_DRIVER_ID);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    static matrix_row_t temp_matrix[MATRIX_ROWS] = {0};

    // Read matrix over SPI
    custom_spi_start(PISO_CS_PIN, false, REGISTERS_SPI_MODE, REGISTERS_SPI_DIV, REGISTERS_SPI_DRIVER_ID);
    custom_spi_receive((uint8_t *)temp_matrix, N_PISO_REGISTERS * sizeof(matrix_row_t), REGISTERS_SPI_DRIVER_ID);
    custom_spi_stop(REGISTERS_SPI_DRIVER_ID);

#if defined(TOUCH_SCREEN_ENABLE) && defined(ONE_HAND_MODE_ENABLE)
    // Do nothing until sensor initialised
    if (ili9341_touch == NULL){
        bool changed = memcmp(current_matrix, temp_matrix, sizeof(temp_matrix)) != 0;
        if (changed) {
            memcpy(current_matrix, temp_matrix, sizeof(temp_matrix));
        }
        return changed;
    }

    touch_report_t touch_report = get_spi_touch_report(ili9341_touch);

    // Convert left-based to center-based coord
    int16_t x = touch_report.x - ILI9341_WIDTH/2;

    // Screen not pressed -> Neither the key
    if (!touch_report.pressed) {
        current_matrix[one_hand_row] &= ~(1 << one_hand_col);
    }
    // Pressed within trigger zone -> "Press" key
    else if (-30 < x && x < 30) {
        current_matrix[one_hand_row] |= 1 << one_hand_col;
    }
#endif // TOUCH_SCREEN_ENABLE && ONE_HAND_MODE_ENABLE

    // Check if we've changed, return the last-read data
    bool changed = memcmp(current_matrix, temp_matrix, sizeof(temp_matrix)) != 0;
    if (changed) {
        memcpy(current_matrix, temp_matrix, sizeof(temp_matrix));
    }
    return changed;
}

#if defined(ONE_HAND_ENABLE)
#    if defined (TOUCH_SCREEN_ENABLE)
void screen_one_hand(touch_report_t touch_report) {
    if (!touch_report.pressed) {
        return;
    }

    int16_t x = touch_report.x - ILI9341_WIDTH/2;
    int16_t y = touch_report.y - ILI9341_HEIGHT/2;

    if (x > 30) {
        one_hand_movement = y > 0 ? DIRECTION_RIGHT : DIRECTION_LEFT;
    } else if (x < -30){
        one_hand_movement = y > 0 ? DIRECTION_UP : DIRECTION_DOWN;
    }
}
#    endif // TOUCH_SCREEN_ENABLE
#endif // ONE_HAND_ENABLE
