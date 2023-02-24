// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// index on the drivers' array

#ifdef SIPO_PINS_DEBUG
#    include <debug.h>
#    include <print.h>
#    include <wait.h>
#    define sipo_dprintf(...) dprintf(__VA_ARGS__)
#else
#    define sipo_dprintf(...) do { } while (0)
#endif

#define get_sipo_bit(byte, bit) ((sipo_pin_state[byte] >> bit) & 0x1)
#define print_sipo_byte(byte)      \
        sipo_dprintf(              \
            "%d%d%d%d%d%d%d%d | ", \
            get_sipo_bit(byte, 0), \
            get_sipo_bit(byte, 1), \
            get_sipo_bit(byte, 2), \
            get_sipo_bit(byte, 3), \
            get_sipo_bit(byte, 4), \
            get_sipo_bit(byte, 5), \
            get_sipo_bit(byte, 6), \
            get_sipo_bit(byte, 7)  \
        )
#define sipo_print_status()                    \
        sipo_dprintf("SIPO status: MCU > ");   \
        for (int i=_SIPO_BYTES-1; i>=0; --i) { \
            print_sipo_byte(i);                \
        }                                      \
        sipo_dprintf("< Chain end\n")

// compute the amount of bytes needed
#define _SIPO_BYTES ((N_SIPO_PINS+7)/8)

extern uint8_t sipo_pin_state[_SIPO_BYTES];

// create pin lists
#define configure_sipo_pins(...)           \
        setPinOutput(SIPO_CS_PIN);         \
        writePinHigh(SIPO_CS_PIN);         \
        enum { __VA_ARGS__, __SIPO_PINS }; \
        _Static_assert(__SIPO_PINS <= N_SIPO_PINS, "Defined more pin names than the amount configured")

// control pins
#define sipo_pin_high(v) set_sipo_pin(v, true)
#define  sipo_pin_low(v) set_sipo_pin(v, false)
void set_sipo_pin(uint8_t position, bool state);

// set status
void write_sipo_state(void);
