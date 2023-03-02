// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/* Notes:
 * - Each register has 8 inputs, aka each row has 8 columns
 * - As such, we can use the number of registers as the size of the variable
 *  !! this would not hold for other registers with a different amount of inputs
 */

#include "quantum.h"
#include "custom_spi_master.h"

#if defined(TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
#    include "touch_driver.h"
#    include "access.h" // declares ili9341_irq
#endif // TOUCH_SCREEN_ENABLE && INIT_EE_HANDS_RIGHT

static inline bool check_changes(matrix_row_t *current_matrix, matrix_row_t *temp_matrix) {
    bool changed = memcmp(current_matrix, temp_matrix, N_PISO_REGISTERS) != 0;
    if (changed) {
        memcpy(current_matrix, temp_matrix, N_PISO_REGISTERS);
    }
    return changed;
}

bool matrix_scan_custom(matrix_row_t *current_matrix) {
    static matrix_row_t temp_matrix[MATRIX_ROWS] = {0};

    // Read matrix over SPI
    custom_spi_start(PISO_CS_PIN, false, REGISTERS_SPI_MODE, REGISTERS_SPI_DIV, REGISTERS_SPI_DRIVER_ID);
    custom_spi_receive((uint8_t *)temp_matrix, N_PISO_REGISTERS, REGISTERS_SPI_DRIVER_ID);
    custom_spi_stop(REGISTERS_SPI_DRIVER_ID);

#if defined(TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
    // IRQ pin is connected to the 1st input of the last shift register
    ili9341_irq = temp_matrix[4] >> 7;

#    if defined(ONE_HAND_MODE_ENABLE)
    // Do nothing until sensor initialised or when screen isn't pressed (IRQ high)
    if (ili9341_touch == NULL || ili9341_irq == true) {
        // Set the selected key's position in the matrix to 0 as screen isn't pressed
        current_matrix[one_hand_row] &= ~(1 << one_hand_col);

        return check_changes(current_matrix, temp_matrix);
    }

    touch_report_t touch_report = get_spi_touch_report(ili9341_touch, false);

    // Convert left-based to center-based coord
    int16_t x = touch_report.x - ILI9341_WIDTH / 2;

    // If screen pressed on the "trigger" area, virtually press selected key
    if (-30 < x && x < 30) {
        current_matrix[one_hand_row] |= 1 << one_hand_col;
    }
#    endif // ONE_HAND_MODE_ENABLE
#endif // TOUCH_SCREEN_ENABLE && INIT_EE_HANDS_RIGHT

    return check_changes(current_matrix, temp_matrix);
}
