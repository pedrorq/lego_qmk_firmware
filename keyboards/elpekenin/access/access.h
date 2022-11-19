// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "qp.h"
extern painter_device_t lcd;
#endif // QUANTUM_PAINTER_ENABLE

#if defined(RGB_MATRIX_ENABLE)
typedef enum key_selector_direction_t{
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} key_selector_direction_t;
extern uint8_t key_selector_mode_last_key;
extern key_selector_direction_t key_selector_direction;
#endif // RGB_MATRIX_ENABLE
