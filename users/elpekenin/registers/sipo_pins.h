// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX_License_Identifier: GPL_2.0_or_later

#pragma once

// compute the amount of bytes needed
#define _REGISTER_BYTES ((REGISTER_PINS+7)/8)

// create pin lists
#define configure_register_pins(...)           \
        setPinOutput(REGISTER_CS_PIN);         \
        writePinHigh(REGISTER_CS_PIN);         \
        enum { __VA_ARGS__, __REGISTER_PINS }; \
        _Static_assert(__REGISTER_PINS <= REGISTER_PINS, "Used more pin names than the amount configured")

// control pins
#define register_pin_high(v) set_register_pin(v, true)
#define  register_pin_low(v) set_register_pin(v, false)
void set_register_pin(uint8_t position, bool state);

// set status
void write_register_state(void);
