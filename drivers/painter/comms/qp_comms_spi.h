// Copyright 2021 Nick Brassel (@tzarc)
// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef QUANTUM_PAINTER_SPI_ENABLE

#    ifdef SIPO_PINS_ENABLE
#        include "sipo_pins.h"
#        include "custom_spi_master.h"
// index on the drivers' array
#        define PAINTER_SPI_DRIVER_ID 0
#        define qp_spi_init() custom_spi_init(PAINTER_SPI_DRIVER_ID)
#        define qp_spi_start(cs, lsb, mode, div) custom_spi_start(DUMMY_PIN, lsb, mode, div, PAINTER_SPI_DRIVER_ID)
#        define qp_spi_write(data) custom_spi_write(data, PAINTER_SPI_DRIVER_ID)
#        define qp_spi_transmit(ptr, bytes) custom_spi_transmit(ptr, bytes, PAINTER_SPI_DRIVER_ID)
#        define qp_spi_stop() custom_spi_stop(PAINTER_SPI_DRIVER_ID)
// only pin we need as output is REGISTER_CS and is already handled by the feature
#        define qp_setPinOutput(pin) do { } while (0)
#        define qp_writePinLow(pin) sipo_pin_low(pin); write_sipo_state()
#        define qp_writePinHigh(pin) sipo_pin_high(pin); write_sipo_state()
#    else // -------------------- regular pin handling
#        include "spi_master.h"
#        define qp_spi_init() spi_init()
#        define qp_spi_start(cs, lsb, mode, div) spi_start(cs, lsb, mode, div)
#        define qp_spi_write(data) spi_write(data)
#        define qp_spi_transmit(ptr, bytes) spi_transmit(ptr, bytes)
#        define qp_spi_stop() spi_stop()
#        define qp_setPinOutput(pin) setPinOutput(pin)
#        define qp_writePinLow(pin) writePinLow(pin)
#        define qp_writePinHigh(pin) writePinHigh(pin)
#    endif // SIPO_PINS_ENABLE

#    include <stdint.h>

#    include "gpio.h"
#    include "qp_internal.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base SPI support

typedef struct qp_comms_spi_config_t {
    pin_t    chip_select_pin;
    uint16_t divisor;
    bool     lsb_first;
    int8_t   mode;
} qp_comms_spi_config_t;

bool     qp_comms_spi_init(painter_device_t device);
bool     qp_comms_spi_start(painter_device_t device);
uint32_t qp_comms_spi_send_data(painter_device_t device, const void* data, uint32_t byte_count);
void     qp_comms_spi_stop(painter_device_t device);

extern const painter_comms_vtable_t spi_comms_vtable;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI with D/C and RST pins

#    ifdef QUANTUM_PAINTER_SPI_DC_RESET_ENABLE

typedef struct qp_comms_spi_dc_reset_config_t {
    qp_comms_spi_config_t spi_config;
    pin_t                 dc_pin;
    pin_t                 reset_pin;
} qp_comms_spi_dc_reset_config_t;

void     qp_comms_spi_dc_reset_send_command(painter_device_t device, uint8_t cmd);
uint32_t qp_comms_spi_dc_reset_send_data(painter_device_t device, const void* data, uint32_t byte_count);
void     qp_comms_spi_dc_reset_bulk_command_sequence(painter_device_t device, const uint8_t* sequence, size_t sequence_len);

extern const painter_comms_with_command_vtable_t spi_comms_with_dc_vtable;

void     qp_comms_spi_dc_reset_single_byte_send_command(painter_device_t device, uint8_t cmd);
uint32_t qp_comms_spi_dc_reset_single_byte_send_data(painter_device_t device, const void* data, uint32_t byte_count);
void     qp_comms_spi_dc_reset_single_byte_bulk_command_sequence(painter_device_t device, const uint8_t* sequence, size_t sequence_len);

extern const painter_comms_with_command_vtable_t spi_comms_with_dc_single_byte_vtable;

#    endif // QUANTUM_PAINTER_SPI_DC_RESET_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // QUANTUM_PAINTER_SPI_ENABLE
