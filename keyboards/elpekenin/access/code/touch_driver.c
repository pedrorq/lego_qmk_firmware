// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>
#include "debug.h"
#include "spi_master.h"
#include "touch_driver.h"

bool touch_spi_init(touch_device_t device) {
    struct touch_driver_t           *driver      = (struct touch_driver_t *)device;
    struct spi_touch_comms_config_t comms_config = driver->spi_config;

    // Initialize the SPI peripheral
    spi_init();

    // Set up CS as output high
    setPinOutput(comms_config.chip_select_pin);
    writePinHigh(comms_config.chip_select_pin);

    // Set up IRQ as input
    setPinInput(comms_config.irq_pin);

    return true;
}

bool touch_spi_start(spi_touch_comms_config_t comms_config) {
    return spi_start(comms_config.chip_select_pin, comms_config.lsb_first, comms_config.mode, comms_config.divisor);
}

void touch_spi_stop(spi_touch_comms_config_t comms_config) {
    spi_stop();
    writePinHigh(comms_config.chip_select_pin);
}

touch_report_t get_spi_touch_report(touch_device_t device) {
    struct touch_driver_t           *driver      = (struct touch_driver_t *)device;
    struct spi_touch_comms_config_t comms_config = driver->spi_config;

    // Static variable so previous report is stored
    // Goal: When the screen is not pressed anymore, we can see the latest point pressed
    static touch_report_t report = {
        .x = 0,
        .y = 0,
        .pressed = false
    };

    if (readPin(comms_config.irq_pin)) {
        report.pressed = false;
        return report;
    }

    if (!touch_spi_start(comms_config)) {
        dprint("Couldn't start touch comms\n");
    }

    report.pressed = true;

    // Read data from sensor, 0-rotation based
    int16_t x = 0;
    int16_t y = 0;

    // Take several measurements and then compute the mean
    for (uint8_t i=0; i<driver->measurements; i++) {
        // Send command
        spi_write(comms_config.x_cmd);
        // Read answer
        x += ((spi_write(0) << 8) | spi_write(0)) >> 3;

        // Send command
        spi_write(comms_config.y_cmd);
        // Read answer
        y += ((spi_write(0) << 8) | spi_write(0)) >> 3;
    }

    // Compute average
    x = x/driver->measurements;
    y = y/driver->measurements;
    dprintf("Average | x: %d, y: %d\n", x, y);

    // Map to correct range
    x = ((x - driver->offset) * driver->width  / driver->scale);
    y = ((y - driver->offset) * driver->height / driver->scale);
    dprintf("Scaled | x: %d, y: %d\n", x, y);

    // Handle posible edge cases
    if (x < 0) { x = 0; }
    if (x > driver->width) { x = driver->width; }
    if (y < 0) { y = 0; }
    if (y > driver->height) { y = driver->height; }

    // Apply updside-down adjustment
    if (driver->upside_down) {
        report.x = driver->width - x;
    }

    // Appropiate type
    uint16_t _x = x;
    uint16_t _y = y;

    // Apply rotation adjustments
    switch (driver->rotation) {
        case TOUCH_ROTATION_0:
            report.x = _x;
            report.y = _y;
            break;

        case TOUCH_ROTATION_90:
            report.x = driver->height - _y;
            report.y = _x;
            break;

        case TOUCH_ROTATION_180:
            report.x = driver->width  - _x;
            report.y = driver->height - _y;
            break;

        case TOUCH_ROTATION_270:
            report.x = _y;
            report.y = driver->width - _x;
            break;
    }

    touch_spi_stop(comms_config);

    return report;
}
