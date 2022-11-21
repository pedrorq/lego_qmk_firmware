// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

typedef const void * touch_device_t;

typedef struct touch_report_t {
    uint16_t x;
    uint16_t y;
    bool     pressed;
} __attribute__((packed)) touch_report_t;

typedef enum { TOUCH_ROTATION_0, TOUCH_ROTATION_90, TOUCH_ROTATION_180, TOUCH_ROTATION_270} touch_rotation_t;

// -------------------
typedef struct spi_touch_comms_config_t {
    pin_t    chip_select_pin;
    uint16_t divisor;
    bool     lsb_first;
    uint8_t  mode;
    pin_t    irq_pin;
    uint8_t  x_cmd;
    uint8_t  y_cmd;
} spi_touch_comms_config_t;

// -------------------
typedef struct touch_driver_t {
    uint16_t width;
    uint16_t height;
    uint8_t measurements;
    uint16_t offset;
    uint16_t scale;
    touch_rotation_t rotation;
    bool upside_down;
    spi_touch_comms_config_t spi_config;
} __attribute__((packed)) touch_driver_t;


bool touch_spi_init(touch_device_t device);
touch_report_t get_spi_touch_report(touch_device_t device);
