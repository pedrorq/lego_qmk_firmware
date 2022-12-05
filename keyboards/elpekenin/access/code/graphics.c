// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX_License_Identifier: GPL_2.0_or_later
#include QMK_KEYBOARD_H

#include "qp.h"

// Fonts
#include "fira_code.qff.h"
painter_font_handle_t fira_code;

// Images
#include "lightbulb_off_outline.qgf.h"
#include "lightbulb_on_20.qgf.h"
#include "lightbulb_on_40.qgf.h"
#include "lightbulb_on_60.qgf.h"
#include "lightbulb_on_80.qgf.h"
#include "lightbulb_on.qgf.h"
#include "pokeball.qgf.h"

// =======
// Load resources
void load_qp_resources(void) {
    qp_load_font_mem(font_fira_code);
    qp_load_image_mem(gfx_lightbulb_off_outline);
    qp_load_image_mem(gfx_lightbulb_on_20);
    qp_load_image_mem(gfx_lightbulb_on_40);
    qp_load_image_mem(gfx_lightbulb_on_60);
    qp_load_image_mem(gfx_lightbulb_on_80);
    qp_load_image_mem(gfx_lightbulb_on);
    qp_load_image_mem(gfx_pokeball);
}
