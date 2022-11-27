// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "qp.h"
extern painter_device_t lcd;
#endif // QUANTUM_PAINTER_ENABLE

#if defined(RGB_MATRIX_ENABLE) && defined(ONE_HAND_ENABLE)
typedef enum one_hand_movement_t{
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} one_hand_movement_t;
extern uint8_t one_hand_col;
extern uint8_t one_hand_row;
extern one_hand_movement_t one_hand_movement;
#endif // RGB_MATRIX_ENABLE && ONE_HAND_ENABLE

#if defined (TOUCH_SCREEN)
#    include "touch_driver.h"
extern touch_device_t touch_device;
#    if defined(ONE_HAND_ENABLE)
void screen_one_hand(touch_report_t touch_report);
#    endif // ONE_HAND_ENABLE
#endif // TOUCH_SCREEN

