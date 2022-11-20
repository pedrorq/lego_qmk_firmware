// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "print.h"
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

bool touch_spi_start(touch_device_t device) {
    struct touch_driver_t       *driver      = (struct touch_driver_t *)device;
    struct touch_comms_config_t comms_config = driver->comms_config;

    return spi_start(comms_config.chip_select_pin, comms_config.lsb_first, comms_config.mode, comms_config.divisor);
}

touch_report_t touch_get_report(touch_device_t device) {
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
        print("IRQ high\n");
        report.pressed = false;
        return report;
    }

    print("IRQ low\n");
    report.pressed = true;

    // Read data from sensor, 0-rotation based
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t buf[2];

    for (uint8_t i=0; i<driver->measurements; i++) {
        // Send command
        writePinLow(comms_config.chip_select_pin);
        spi_write(0xD0);
        writePinHigh(comms_config.chip_select_pin);

        // Receive answer
        spi_receive(buf, 2);

        // Parse data
        x += ((buf[0]<<8) + buf[1])>>3;
        // ------------------------------------
        // Send command
        writePinLow(comms_config.chip_select_pin);
        spi_write(0x90);
        writePinHigh(comms_config.chip_select_pin);

        // Receive answer
        spi_receive(buf, 2);

        // Parse data
        y += ((buf[0]<<8) + buf[1])>>3;
    }

    x = (uint16_t) x/driver->measurements;
    y = (uint16_t) y/driver->measurements;

    // This was on the Python code, not seen it on C's example code (didn't look much tbh)
    // x = (uint16_t) ((x - driver->offset) * driver->width/driver->max);
    // y = (uint16_t) ((y - driver->offset) * driver->height/driver->max);

    // Apply rotation adjustments
    switch (driver->rotation) {
        case TOUCH_ROTATION_0:
            report.x = x;
            report.y = y;
            break;

        case TOUCH_ROTATION_90:
            report.x = driver->height - y;
            report.y = x;
            break;

        case TOUCH_ROTATION_180:
            report.x = driver->width  - x;
            report.y = driver->height - y;
            break;

        case TOUCH_ROTATION_270:
            report.x = y;
            report.y = driver->width - x;
            break;
    }

    //IRQ stays at low without this, makes no sense as:
    // 1- The loop already ends with CS set on high
    // 2- We are (according to prints) not getting here
    writePinHigh(comms_config.chip_select_pin);

    return report;
}
