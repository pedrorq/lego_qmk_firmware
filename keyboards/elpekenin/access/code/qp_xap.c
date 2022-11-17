// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

// === This file exposes QP **drawing** functions over XAP, in the same order they appear at https://docs.qmk.fm/#/quantum_painter?id=quantum-painter-api-primitives
// TODO
// - Maybe the XAP answer should be success/failure based on the output from QP functions, returning success for now as in "your request has been handled"
// - Animations, I guess they need to be checked

#include QMK_KEYBOARD_H

#include "qp.h"
#include <xap.h>

// ===========================================================
// Helpers
uint16_t get_u16(const uint8_t *data, uint8_t *i) {
    // increase the counter so it is already aligned with the next element
    *i += 2;

    // data position based on incremented i
    return (data[*i-1] << 8 | data[*i-2]);
}

uint8_t get_u8(const uint8_t *data, uint8_t *i) {
    // increase the counter so it is already aligned with the next element
    *i += 1;

    // data position based on incremented i
    return data[*i-1];
}

void xap_respond_success(xap_token_t token); // Weird stuff to get the function imported

#define CHECK_DATA_LEN_IS(len) \
    if (data_len != len) { \
        qp_dprintf("---\nERROR: QP over XAP check failed, got %dbytes while expecting %dbytes\n---\n", len, data_len); \
        return false; \
    }

#define CHECK_DATA_LEN_IS_AT_LEAST(len) \
    if (data_len < len) { \
        qp_dprintf("---\nERROR: QP over XAP check failed, got %dbytes while expecting at least %dbytes\n---\n", len, data_len); \
        return false; \
    }


// ===========================================================
// Handlers
bool xap_respond_qp_clear(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(1)

    uint8_t counter = 0; uint8_t *i = &counter;
    painter_device_t dev = qp_xap_displays[get_u8(data, i)];

    return qp_clear(dev);
}

bool xap_respond_qp_setpixel(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(8)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t dev = qp_xap_displays[get_u8(data, i)];
    uint16_t         x   = get_u16(data, i);
    uint16_t         y   = get_u16(data, i);
    uint8_t          hue = get_u8(data, i);
    uint8_t          sat = get_u8(data, i);
    uint8_t          val = get_u8(data, i);

    xap_respond_success(token);
    return qp_setpixel(dev, x, y, hue, sat, val);
}

bool xap_respond_qp_line(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(12)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t dev = qp_xap_displays[get_u8(data, i)];
    uint16_t x0  = get_u16(data, i);
    uint16_t y0  = get_u16(data, i);
    uint16_t x1  = get_u16(data, i);
    uint16_t y1  = get_u16(data, i);
    uint8_t  hue = get_u8(data, i);
    uint8_t  sat = get_u8(data, i);
    uint8_t  val = get_u8(data, i);

    xap_respond_success(token);
    return qp_line(dev, x0, y0, x1, y1, hue, sat, val);

}

bool xap_respond_qp_rect(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(13)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t dev = qp_xap_displays[get_u8(data, i)];
    uint16_t left   = get_u16(data, i);
    uint16_t top    = get_u16(data, i);
    uint16_t right  = get_u16(data, i);
    uint16_t bottom = get_u16(data, i);
    uint8_t  hue    = get_u8(data, i);
    uint8_t  sat    = get_u8(data, i);
    uint8_t  val    = get_u8(data, i);
    bool     filled = get_u8(data, i);

    xap_respond_success(token);
    return qp_rect(dev, left, top, right, bottom, hue, sat, val, filled);
}

bool xap_respond_qp_circle(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(11)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t dev = qp_xap_displays[get_u8(data, i)];
    uint16_t x      = get_u16(data, i);
    uint16_t y      = get_u16(data, i);
    uint16_t radius = get_u16(data, i);
    uint8_t  hue    = get_u8(data, i);
    uint8_t  sat    = get_u8(data, i);
    uint8_t  val    = get_u8(data, i);
    bool     filled = get_u8(data, i);

    xap_respond_success(token);
    return qp_circle(dev, x, y, radius, hue, sat, val, filled);

}

bool xap_respond_qp_ellipse(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(13)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t dev = qp_xap_displays[get_u8(data, i)];
    uint16_t x      = get_u16(data, i);
    uint16_t y      = get_u16(data, i);
    uint16_t sizex  = get_u16(data, i);
    uint16_t sizey  = get_u16(data, i);
    uint8_t  hue    = get_u8(data, i);
    uint8_t  sat    = get_u8(data, i);
    uint8_t  val    = get_u8(data, i);
    bool     filled = get_u8(data, i);

    xap_respond_success(token);
    return qp_ellipse(dev, x, y, sizex, sizey, hue, sat, val, filled);
}

bool xap_respond_qp_drawimage(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(6)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t       dev = qp_xap_displays[get_u8(data, i)];
    uint16_t               x   = get_u16(data, i);
    uint16_t               y   = get_u16(data, i);
    painter_image_handle_t img = qp_xap_images[get_u8(data, i)];

    return qp_drawimage(dev, x, y, img);
}

bool xap_respond_qp_drawimage_recolor(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(12)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t       dev    = qp_xap_displays[get_u8(data, i)];
    uint16_t               x      = get_u16(data, i);
    uint16_t               y      = get_u16(data, i);
    painter_image_handle_t img    = qp_xap_images[get_u8(data, i)];
    uint8_t                hue_fg = get_u8(data, i);
    uint8_t                sat_fg = get_u8(data, i);
    uint8_t                val_fg = get_u8(data, i);
    uint8_t                hue_bg = get_u8(data, i);
    uint8_t                sat_bg = get_u8(data, i);
    uint8_t                val_bg = get_u8(data, i);

    return qp_drawimage_recolor(dev, x, y, img, hue_fg, sat_fg, val_fg, hue_bg, sat_bg, val_bg);
}

bool xap_respond_qp_animate(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(6)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t       dev = qp_xap_displays[get_u8(data, i)];
    uint16_t               x   = get_u16(data, i);
    uint16_t               y   = get_u16(data, i);
    painter_image_handle_t img = qp_xap_images[get_u8(data, i)];

    qp_animate(dev, x, y, img);
    return true; //TODO check this
}

bool xap_respond_qp_animate_recolor(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS(12)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t       dev    = qp_xap_displays[get_u8(data, i)];
    uint16_t               x      = get_u16(data, i);
    uint16_t               y      = get_u16(data, i);
    painter_image_handle_t img    = qp_xap_images[get_u8(data, i)];
    uint8_t                hue_fg = get_u8(data, i);
    uint8_t                sat_fg = get_u8(data, i);
    uint8_t                val_fg = get_u8(data, i);
    uint8_t                hue_bg = get_u8(data, i);
    uint8_t                sat_bg = get_u8(data, i);
    uint8_t                val_bg = get_u8(data, i);

    qp_animate_recolor(dev, x, y, img, hue_fg, sat_fg, val_fg, hue_bg, sat_bg, val_bg);
    return true; //TODO check this
}

bool xap_respond_qp_drawtext(xap_token_t token, const uint8_t *data, size_t data_len) {
    CHECK_DATA_LEN_IS_AT_LEAST(6)

    uint8_t counter = 0; uint8_t *i = &counter;

    painter_device_t       dev  = qp_xap_displays[get_u8(data, i)];
    uint16_t               x    = get_u16(data, i);
    uint16_t               y    = get_u16(data, i);
    painter_font_handle_t  font = qp_xap_fonts[get_u8(data, i)];

    qp_drawtext(dev, x, y, font,  (char *) &data[counter]);
    return true; //TODO check this
}
