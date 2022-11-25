// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include QMK_KEYBOARD_H

#include "qp.h"

extern painter_font_handle_t fira_code;
extern painter_image_handle_t message;
extern painter_image_handle_t thermometer;

void load_qp_resources(void);
