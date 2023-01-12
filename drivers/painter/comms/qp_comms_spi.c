// Copyright 2021 Nick Brassel (@tzarc)
// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef QUANTUM_PAINTER_SPI_ENABLE

#    include "qp_comms_spi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base SPI support

#    ifdef SIPO_PINS
#        include "custom_spi_master.h"
#        include "sipo_pins.h"
#        define PAINTER_SPI_DRIVER_ID 0
#        define qp_spi_init() custom_spi_init(PAINTER_SPI_DRIVER_ID)
#        define qp_spi_start(cs, lsb, mode, div) custom_spi_start(DUMMY_PIN, lsb, mode, div, PAINTER_SPI_DRIVER_ID)
#        define qp_spi_write(data) custom_spi_write(data, PAINTER_SPI_DRIVER_ID)
#        define qp_spi_transmit(ptr, bytes) custom_spi_transmit(ptr, bytes, PAINTER_SPI_DRIVER_ID)
#        define qp_spi_stop() custom_spi_stop(PAINTER_SPI_DRIVER_ID)
         // only pin we need as output is REGISTER_CS and is already handled by the feature
#        define qp_setPinOutput(pin) do { } while (0)
#        define qp_writePinLow(pin) register_pin_low(pin); write_register_state()
#        define qp_writePinHigh(pin) register_pin_high(pin); write_register_state()
#    else
         // regular pin handling
#        include "spi_master.h"
#        define qp_spi_init() spi_init()
#        define qp_spi_start(cs, lsb, mode, div) spi_start(cs, lsb, mode, div)
#        define qp_spi_write(data) spi_write(data)
#        define qp_spi_transmit(ptr, bytes) spi_transmit(ptr, bytes)
#        define qp_spi_stop() spi_stop()
#        define qp_setPinOutput(pin) setPinOutput(pin)
#        define qp_writePinLow(pin) writePinLow(pin)
#        define qp_writePinHigh(pin) writePinHigh(pin)
#    endif

bool qp_comms_spi_init(painter_device_t device) {
    struct painter_driver_t *     driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_config_t *comms_config = (struct qp_comms_spi_config_t *)driver->comms_config;

    // Initialize the SPI peripheral
    qp_spi_init();

    // Set up CS as output high
    qp_setPinOutput(comms_config->chip_select_pin);
    qp_writePinHigh(comms_config->chip_select_pin);

    return true;
}

bool qp_comms_spi_start(painter_device_t device) {
    struct painter_driver_t *     driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_config_t *comms_config = (struct qp_comms_spi_config_t *)driver->comms_config;
    return qp_spi_start(comms_config->chip_select_pin, comms_config->lsb_first, comms_config->mode, comms_config->divisor);
}

uint32_t qp_comms_spi_send_data(painter_device_t device, const void *data, uint32_t byte_count) {
    uint32_t       bytes_remaining = byte_count;
    const uint8_t *p               = (const uint8_t *)data;
    uint32_t       max_msg_length  = 1024;

    while (bytes_remaining > 0) {
        uint32_t bytes_this_loop = QP_MIN(bytes_remaining, max_msg_length);
        qp_spi_transmit(p, bytes_this_loop);
        p += bytes_this_loop;
        bytes_remaining -= bytes_this_loop;
    }

    return byte_count - bytes_remaining;
}

void qp_comms_spi_stop(painter_device_t device) {
    struct painter_driver_t *     driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_config_t *comms_config = (struct qp_comms_spi_config_t *)driver->comms_config;
    qp_spi_stop();
    qp_writePinHigh(comms_config->chip_select_pin);
}

const struct painter_comms_vtable_t spi_comms_vtable = {
    .comms_init  = qp_comms_spi_init,
    .comms_start = qp_comms_spi_start,
    .comms_send  = qp_comms_spi_send_data,
    .comms_stop  = qp_comms_spi_stop,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI with D/C and RST pins

#    ifdef QUANTUM_PAINTER_SPI_DC_RESET_ENABLE

bool qp_comms_spi_dc_reset_init(painter_device_t device) {
    if (!qp_comms_spi_init(device)) {
        return false;
    }

    struct painter_driver_t *              driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_dc_reset_config_t *comms_config = (struct qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    // Set up D/C as output low, if specified
#ifndef SIPO_PINS
    if (comms_config->dc_pin != NO_PIN) {
#endif
        qp_setPinOutput(comms_config->dc_pin);
        qp_writePinLow(comms_config->dc_pin);
#ifndef SIPO_PINS
    }
#endif

    // Set up RST as output, if specified, performing a reset in the process
#ifndef SIPO_PINS
    if (comms_config->reset_pin != NO_PIN) {
#endif
        qp_setPinOutput(comms_config->reset_pin);
        qp_writePinLow(comms_config->reset_pin);
        wait_ms(20);
        qp_writePinHigh(comms_config->reset_pin);
        wait_ms(20);
#ifndef SIPO_PINS
    }
#endif

    return true;
}

uint32_t qp_comms_spi_dc_reset_send_data(painter_device_t device, const void *data, uint32_t byte_count) {
    struct painter_driver_t *              driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_dc_reset_config_t *comms_config = (struct qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    qp_writePinHigh(comms_config->dc_pin);

#ifdef SIPO_PINS
    qp_writePinLow(comms_config->spi_config.chip_select_pin);
#endif

    uint32_t ret = qp_comms_spi_send_data(device, data, byte_count);

#ifdef SIPO_PINS
    qp_writePinHigh(comms_config->spi_config.chip_select_pin);
#endif

    return ret;
}

void qp_comms_spi_dc_reset_send_command(painter_device_t device, uint8_t cmd) {
    struct painter_driver_t *              driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_dc_reset_config_t *comms_config = (struct qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    qp_writePinLow(comms_config->dc_pin);

#ifdef SIPO_PINS
    qp_writePinLow(comms_config->spi_config.chip_select_pin);
#endif

    qp_spi_write(cmd);

#ifdef SIPO_PINS
    qp_writePinHigh(comms_config->spi_config.chip_select_pin);
#endif
}

void qp_comms_spi_dc_reset_bulk_command_sequence(painter_device_t device, const uint8_t *sequence, size_t sequence_len) {
    for (size_t i = 0; i < sequence_len;) {
        uint8_t command   = sequence[i];
        uint8_t delay     = sequence[i + 1];
        uint8_t num_bytes = sequence[i + 2];
        qp_comms_spi_dc_reset_send_command(device, command);
        if (num_bytes > 0) {
            qp_comms_spi_dc_reset_send_data(device, &sequence[i + 3], num_bytes);
        }
        if (delay > 0) {
            wait_ms(delay);
        }
        i += (3 + num_bytes);
    }
}

const struct painter_comms_with_command_vtable_t spi_comms_with_dc_vtable = {
    .base =
        {
            .comms_init  = qp_comms_spi_dc_reset_init,
            .comms_start = qp_comms_spi_start,
            .comms_send  = qp_comms_spi_dc_reset_send_data,
            .comms_stop  = qp_comms_spi_stop,
        },
    .send_command          = qp_comms_spi_dc_reset_send_command,
    .bulk_command_sequence = qp_comms_spi_dc_reset_bulk_command_sequence,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI with D/C and RST pins but sending one byte at a time, needed for some devices

void qp_comms_spi_dc_reset_single_byte_send_command(painter_device_t device, uint8_t cmd) {
    struct painter_driver_t *              driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_dc_reset_config_t *comms_config = (struct qp_comms_spi_dc_reset_config_t *)driver->comms_config;
    qp_writePinLow(comms_config->spi_config.chip_select_pin);
    qp_comms_spi_dc_reset_send_command(device, cmd);
    qp_writePinHigh(comms_config->spi_config.chip_select_pin);
}

uint32_t qp_comms_spi_dc_reset_single_byte_send_data(painter_device_t device, const void *data, uint32_t byte_count) {
    struct painter_driver_t *              driver       = (struct painter_driver_t *)device;
    struct qp_comms_spi_dc_reset_config_t *comms_config = (struct qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    uint32_t       bytes_remaining = byte_count;
    const uint8_t *p               = (const uint8_t *)data;
    uint32_t       max_msg_length  = 1;

    qp_writePinHigh(comms_config->dc_pin);
    while (bytes_remaining > 0) {
        uint32_t bytes_this_loop = QP_MIN(bytes_remaining, max_msg_length);
        qp_writePinLow(comms_config->spi_config.chip_select_pin);
        qp_spi_transmit(p, bytes_this_loop);
        qp_writePinHigh(comms_config->spi_config.chip_select_pin);
        p += bytes_this_loop;
        bytes_remaining -= bytes_this_loop;
    }

    return byte_count - bytes_remaining;
}

void qp_comms_spi_dc_reset_single_byte_bulk_command_sequence(painter_device_t device, const uint8_t *sequence, size_t sequence_len) {
    for (size_t i = 0; i < sequence_len;) {
        uint8_t command   = sequence[i];
        uint8_t delay     = sequence[i + 1];
        uint8_t num_bytes = sequence[i + 2];
        qp_comms_spi_dc_reset_single_byte_send_command(device, command);
        if (num_bytes > 0) {
            qp_comms_spi_dc_reset_single_byte_send_data(device, &sequence[i + 3], num_bytes);
        }
        if (delay > 0) {
            wait_ms(delay);
        }
        i += (3 + num_bytes);
    }
}

const struct painter_comms_with_command_vtable_t spi_comms_with_dc_single_byte_vtable = {
    .base =
        {
            .comms_init  = qp_comms_spi_dc_reset_init,
            .comms_start = qp_comms_spi_start,
            .comms_send  = qp_comms_spi_dc_reset_single_byte_send_data,
            .comms_stop  = qp_comms_spi_stop,
        },
    .send_command          = qp_comms_spi_dc_reset_single_byte_send_command,
    .bulk_command_sequence = qp_comms_spi_dc_reset_single_byte_bulk_command_sequence,
};
#    endif // QUANTUM_PAINTER_SPI_DC_RESET_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // QUANTUM_PAINTER_SPI_ENABLE
