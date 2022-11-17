// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

#include "qp.h"

// Images
#include "rp2040.qgf.h"
static painter_image_handle_t rp2040;

// Fonts
#include "fira_code.qff.h"
static painter_font_handle_t fira_code;

// =======
// LCD Init
void load_qp_resources(void) {
    rp2040 = qp_load_image_mem(gfx_rp2040);
    fira_code = qp_load_font_mem(font_fira_code);
}
