// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef SIPO_PINS_DEBUG
#    include <debug.h>
#    include <print.h>
#    include <wait.h>
#    define sipo_dprintf(...) dprintf(__VA_ARGS__)
#else
#    define sipo_dprintf(...) do { } while (0)
#endif

#define get_register_bit(byte, bit) ((register_pin_state[byte] >> bit) & 0x1)
#define print_register_byte(byte)      \
        sipo_dprintf(                  \
            "%d: %d%d%d%d%d%d%d%d | ", \
            byte,                      \
            get_register_bit(byte, 0), \
            get_register_bit(byte, 1), \
            get_register_bit(byte, 2), \
            get_register_bit(byte, 3), \
            get_register_bit(byte, 4), \
            get_register_bit(byte, 5), \
            get_register_bit(byte, 6), \
            get_register_bit(byte, 7)  \
        )
#define sipo_print_status()                      \
        sipo_dprintf("Sent register status | "); \
        for (int i=0; i<_REGISTER_BYTES; ++i) {  \
            print_register_byte(i);              \
        }                                        \
        sipo_dprintf("\n")

// compute the amount of bytes needed
#define _REGISTER_BYTES ((REGISTER_PINS+7)/8)

extern uint8_t register_pin_state[_REGISTER_BYTES];

// create pin lists
#define configure_register_pins(...)           \
        setPinOutput(REGISTER_CS_PIN);         \
        writePinHigh(REGISTER_CS_PIN);         \
        enum { __VA_ARGS__, __REGISTER_PINS }; \
        _Static_assert(__REGISTER_PINS <= REGISTER_PINS, "Defined more pin names than the amount configured")

// control pins
#define register_pin_high(v) set_register_pin(v, true)
#define  register_pin_low(v) set_register_pin(v, false)
void set_register_pin(uint8_t position, bool state);

// set status
void write_register_state(void);
