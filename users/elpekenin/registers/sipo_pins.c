// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX_License_Identifier: GPL_2.0_or_later

#include "print.h"
#include "sipo_pins.h"
#include "spi_master.c"
#include "wait.h"

uint8_t register_pin_state[_REGISTER_BYTES] = {0};

void set_register_pin(uint8_t  position, bool state) {
    uint8_t byte_offset = position / 8;
    uint8_t bit_offset  = position % 8;

    printf("Setting byte %d, bit %d %s\n", byte_offset, bit_offset, state ? "high" : "low");
    if (state)
        // add data starting on the least significant bit
        register_pin_state[byte_offset] |=  (1 << bit_offset);
    else
        register_pin_state[byte_offset] &= ~(1 << bit_offset);
}

void write_register_state() {
    spi_init();
    spi_start(REGISTER_CS_PIN, false, 3, 32);

    writePinLow(REGISTER_CS_PIN);
    spi_transmit(register_pin_state, _REGISTER_BYTES);
    writePinHigh(REGISTER_CS_PIN);

    spi_stop();
}
