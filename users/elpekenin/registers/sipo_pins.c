// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sipo_pins.h"
#include "custom_spi_master.h"

#define REGISTER_SPI_DRIVER_ID 1

uint8_t register_pin_state[_REGISTER_BYTES] = {[0 ... _REGISTER_BYTES-1] = 0};
bool register_state_changed = true;

void set_register_pin(uint8_t  position, bool state) {
    // this change makes position 0 to be the closest to the MCU, instead of being the 1st bit of the last byte
    uint8_t byte_offset = _REGISTER_BYTES - 1 - (position / 8);
    uint8_t bit_offset  = position % 8;

    // Check if pin already had that state
    uint8_t curr_value = (register_pin_state[byte_offset] >> bit_offset) & 1;
    if (curr_value == state) {
        return;
    }

    register_state_changed = true;

    if (state)
        // add data starting on the least significant bit
        register_pin_state[byte_offset] |=  (1 << bit_offset);
    else
        register_pin_state[byte_offset] &= ~(1 << bit_offset);
}

void write_register_state() {
    if (!register_state_changed) {
        return;
    }

    register_state_changed = false;

    custom_spi_init(REGISTER_SPI_DRIVER_ID);

    if(!custom_spi_start(REGISTER_CS_PIN, false, REGISTER_SPI_MODE, REGISTER_SPI_DIV, REGISTER_SPI_DRIVER_ID)) {
        sipo_dprintf("Couldn't start SPI for SIPO\n");
        return;
    }

    writePinLow(REGISTER_CS_PIN);
    custom_spi_transmit(register_pin_state, _REGISTER_BYTES, REGISTER_SPI_DRIVER_ID);
    writePinHigh(REGISTER_CS_PIN);

    custom_spi_stop(REGISTER_SPI_DRIVER_ID);
}
