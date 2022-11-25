// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

#include "qp.h"

// Images
#include "thermometer.qgf.h"
painter_image_handle_t thermometer;
#include "message.qgf.h"
painter_image_handle_t message;

// Fonts
#include "fira_code.qff.h"
painter_font_handle_t fira_code;

// =======
// LCD Init
void load_qp_resources(void) {
    thermometer = qp_load_image_mem(gfx_thermometer);
    message = qp_load_image_mem(gfx_message);
    fira_code = qp_load_font_mem(font_fira_code);
}
