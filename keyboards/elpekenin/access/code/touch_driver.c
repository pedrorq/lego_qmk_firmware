// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "debug.h"
#include "spi_master.h"
#include "touch_driver.h"

bool touch_spi_init(touch_device_t device) {
    struct touch_driver_t       *driver      = (struct touch_driver_t *)device;
    struct touch_comms_config_t comms_config = driver->comms_config;

    // Initialize the SPI peripheral
    spi_init();

    // Set up CS as output high
    setPinOutput(comms_config.chip_select_pin);
    writePinHigh(comms_config.chip_select_pin);

    // Set up IRQ as input
    setPinInput(comms_config.irq_pin);

    return true;
}

bool touch_spi_start(touch_comms_config_t comms_config) {
    return spi_start(comms_config.chip_select_pin, comms_config.lsb_first, comms_config.mode, comms_config.divisor);
}

void touch_spi_stop(touch_comms_config_t comms_config) {
    spi_stop();
    writePinHigh(comms_config.chip_select_pin);
}

touch_report_t get_touch_report(touch_device_t device) {
    struct touch_driver_t       *driver      = (struct touch_driver_t *)device;
    struct touch_comms_config_t comms_config = driver->comms_config;

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
    uint8_t buf[2];

    // Take several measurements and then compute the mean
    for (uint8_t i=0; i<driver->measurements; i++) {
        // Send command
        spi_write(0xD0);
        // Receive answer
        spi_receive(buf, 2);
        // Parse data
        x += ((buf[0]<<8) + buf[1])>>3;

        // Send command
        spi_write(0x90);
        // Receive answer
        spi_receive(buf, 2);
        // Parse data
        y += ((buf[0]<<8) + buf[1])>>3;
    }

    // Compute average
    x = x/driver->measurements;
    y = y/driver->measurements;

    // Map to correct range
    x = (driver->offset_x + x * driver->scale_x);
    y = (driver->offset_y + y * driver->scale_y);

    // Handle posible edge cases
    if (x < 0) { x = 0; }
    if (x > driver->width) { x = driver->width; }
    if (y < 0) { y = 0; }
    if (y > driver->height) { y = driver->height; }

    // Convert to appropiate types:
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
