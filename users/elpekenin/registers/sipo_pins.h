// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX_License_Identifier: GPL_2.0_or_later

#pragma once

// create pin lists
#define configure_register_pins(...) enum { __VA_ARGS__, __REGISTER_PINS }

// control pins
#define register_pin_high(v) set_register_pin(v, true)
#define  register_pin_low(v) set_register_pin(v, false)
void set_register_pin(uint8_t position, bool state);

// set status
// we need this weird wrapper as the compiler doesn't know what `__REGISTER_PINS` is when inspecting the function
#define write_register_state() _write_register_state(__REGISTER_PINS)

