// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "gpio.h"
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

void touch_spi_send(touch_device_t device, const uint8_t *data, uint8_t byte_count) {
    struct touch_driver_t       *driver      = (struct touch_driver_t *)device;
    struct touch_comms_config_t comms_config = driver->comms_config;

    // Use CS so the data is received by touch screen
    writePinLow(comms_config.chip_select_pin);
    spi_transmit(data, byte_count);
    writePinHigh(comms_config.chip_select_pin);
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

    if (readPin(comms_config.irq_pin) != 0) {
        report.pressed = false;
        return report;
    }

    report.pressed = true;

    // Read data from sensor, 0-rotation based
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t buf[2];

    for (uint8_t i=0; i<3; i++) {
        touch_spi_send(device, (uint8_t *)0xD0, 1);
        spi_receive(buf, 2);
        x += ((buf[0]<<8) + buf[1])>>3;

        touch_spi_send(device, (uint8_t *)0x90, 1);
        spi_receive(buf, 2);
        y += ((buf[0]<<8) + buf[1])>>3;
    }

    x = (uint16_t) ((x/3-driver->offset) * driver->width/driver->max);
    y = (uint16_t) ((y/3-driver->offset) * driver->width/driver->max);

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

    return report;
}
