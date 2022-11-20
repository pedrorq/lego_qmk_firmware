// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

typedef enum { TOUCH_ROTATION_0, TOUCH_ROTATION_90, TOUCH_ROTATION_180, TOUCH_ROTATION_270} touch_rotation_t;

typedef struct touch_comms_config_t {
    pin_t    chip_select_pin;
    uint16_t divisor;
    bool     lsb_first;
    uint8_t  mode;
    pin_t    irq_pin;
} touch_comms_config_t;

typedef struct touch_driver_t {
    uint16_t width;
    uint16_t height;
    uint8_t measurements;
    uint16_t offset;
    uint16_t max;
    touch_rotation_t rotation;
    touch_comms_config_t comms_config;
} __attribute__((packed)) touch_driver_t;

typedef const void * touch_device_t;

typedef struct touch_report_t {
    uint16_t x;
    uint16_t y;
    bool     pressed;
} __attribute__((packed)) touch_report_t;

bool touch_spi_init(touch_device_t device);
bool touch_spi_start(touch_device_t device);
touch_report_t touch_get_report(touch_device_t device);
