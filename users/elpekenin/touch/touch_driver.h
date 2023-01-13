// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef TOUCH_SCREEN_DEBUG
#    include <debug.h>
#    include <print.h>
#    define ts_dprintf(...) dprintf(__VA_ARGS__)
#else
#    define ts_dprintf(...) do { } while (0)
#endif

#    ifdef SIPO_PINS
#        include "custom_spi_master.h"
#        include "sipo_pins.h"
#        define TOUCH_SPI_DRIVER_ID 0
#        define ts_spi_init() custom_spi_init(TOUCH_SPI_DRIVER_ID)
#        define ts_spi_start(cs, lsb, mode, div) custom_spi_start(DUMMY_PIN, lsb, mode, div, TOUCH_SPI_DRIVER_ID)
#        define ts_spi_write(data) custom_spi_write(data, TOUCH_SPI_DRIVER_ID)
#        define ts_spi_transmit(ptr, bytes) custom_spi_transmit(ptr, bytes, TOUCH_SPI_DRIVER_ID)
#        define ts_spi_stop() custom_spi_stop(TOUCH_SPI_DRIVER_ID)
         // only pin we need as output is REGISTER_CS and is already handled by the feature
#        define ts_setPinOutput(pin) do { } while (0)
#        define ts_writePinLow(pin) register_pin_low(pin); write_register_state()
#        define ts_writePinHigh(pin) register_pin_high(pin); write_register_state()
#    else
         // regular pin handling
#        include "spi_master.h"
#        define ts_spi_init() spi_init()
#        define ts_spi_start(cs, lsb, mode, div) spi_start(cs, lsb, mode, div)
#        define ts_spi_write(data) spi_write(data)
#        define ts_spi_transmit(ptr, bytes) spi_transmit(ptr, bytes)
#        define ts_spi_stop() spi_stop()
#        define ts_setPinOutput(pin) setPinOutput(pin)
#        define ts_writePinLow(pin) writePinLow(pin)
#        define ts_writePinHigh(pin) writePinHigh(pin)
#    endif

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
} __attribute__((packed)) spi_touch_comms_config_t;

// -------------------
typedef struct touch_driver_t {
    uint16_t width;
    uint16_t height;
    uint8_t measurements;
    float scale_x;
    float scale_y;
    int16_t offset_x;
    int16_t offset_y;
    touch_rotation_t rotation;
    bool upside_down;
    spi_touch_comms_config_t spi_config;
} __attribute__((packed)) touch_driver_t;


bool touch_spi_init(touch_device_t device);
touch_report_t get_spi_touch_report(touch_device_t device);
