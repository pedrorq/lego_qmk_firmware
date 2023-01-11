// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "print.h"
#include "sipo_pins.h"
#include "custom_spi_master.h"
#include "wait.h"

#define REGISTER_SPI_DRIVER_ID 1

uint8_t register_pin_state[_REGISTER_BYTES] = {[0 ... _REGISTER_BYTES-1] = 0};

void set_register_pin(uint8_t  position, bool state) {
    uint8_t byte_offset = position / 8;
    uint8_t bit_offset  = position % 8;

    if (state)
        // add data starting on the least significant bit
        register_pin_state[byte_offset] |=  (1 << bit_offset);
    else
        register_pin_state[byte_offset] &= ~(1 << bit_offset);
}

void write_register_state() {
    custom_spi_init(REGISTER_SPI_DRIVER_ID);

    if(!custom_spi_start(REGISTER_CS_PIN, false, REGISTER_SPI_MODE, REGISTER_SPI_DIV, REGISTER_SPI_DRIVER_ID)) {
        printf("Couldn't start SPI for SIPO\n");
        return;
    }


    writePinLow(REGISTER_CS_PIN);
    custom_spi_transmit(register_pin_state, _REGISTER_BYTES, REGISTER_SPI_DRIVER_ID);
    writePinHigh(REGISTER_CS_PIN);

    custom_spi_stop(REGISTER_SPI_DRIVER_ID);
}
