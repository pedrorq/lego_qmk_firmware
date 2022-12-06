// Copyright 2022 Pablo Martinez (@elpekenin)
// Copyright 2021 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "color.h"
#include "qp_internal.h"
#include "qp_comms.h"
#include "qp_draw.h"
#include "qp_eink_panel.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantum Painter API implementations

// Power control
bool qp_eink_panel_power(painter_device_t device, bool power_on) {
    struct painter_driver_t *                          driver = (struct painter_driver_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->driver_vtable;
    qp_comms_command(device, power_on ? vtable->opcodes.display_on : vtable->opcodes.display_off);
    return true;
}

// Screen clear
bool qp_eink_panel_clear(painter_device_t device) {
    struct painter_driver_t *driver = (struct painter_driver_t *)device;
    driver->driver_vtable->init(device, driver->rotation); // Re-init the LCD
    return true;
}

// Screen flush
bool qp_eink_panel_flush(painter_device_t device) {
    struct painter_driver_t *driver = (struct painter_driver_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->driver_vtable;
    qp_comms_command(device, vtable->opcodes.refresh);
    return true;
}

// Stream pixel data to the current write position in GRAM
bool qp_eink_panel_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct painter_driver_t *driver = (struct painter_driver_t *)device;
    qp_comms_send(device, pixel_data, native_pixel_count * driver->native_bits_per_pixel / 8);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert supplied palette entries into their native equivalents

bool qp_eink_panel_palette_convert_rgb565_swapped(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    for (int16_t i = 0; i < palette_size; ++i) {
        RGB      rgb      = hsv_to_rgb_nocie((HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v});
        uint16_t rgb565   = (((uint16_t)rgb.r) >> 3) << 11 | (((uint16_t)rgb.g) >> 2) << 5 | (((uint16_t)rgb.b) >> 3);
        palette[i].rgb565 = __builtin_bswap16(rgb565);
    }
    return true;
}

bool qp_eink_panel_palette_convert_rgb888(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    for (int16_t i = 0; i < palette_size; ++i) {
        RGB rgb             = hsv_to_rgb_nocie((HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v});
        palette[i].rgb888.r = rgb.r;
        palette[i].rgb888.g = rgb.g;
        palette[i].rgb888.b = rgb.b;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Append pixels to the target location, keyed by the pixel index

bool qp_eink_panel_append_pixels_rgb565(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    uint16_t *buf = (uint16_t *)target_buffer;
    for (uint32_t i = 0; i < pixel_count; ++i) {
        buf[pixel_offset + i] = palette[palette_indices[i]].rgb565;
    }
    return true;
}

bool qp_eink_panel_append_pixels_rgb888(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    for (uint32_t i = 0; i < pixel_count; ++i) {
        target_buffer[(pixel_offset + i) * 3 + 0] = palette[palette_indices[i]].rgb888.r;
        target_buffer[(pixel_offset + i) * 3 + 1] = palette[palette_indices[i]].rgb888.g;
        target_buffer[(pixel_offset + i) * 3 + 2] = palette[palette_indices[i]].rgb888.b;
    }
    return true;
}
