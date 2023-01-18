// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_comms.h"
#include "qp_draw.h"
#include "qp_eink_panel.h"
#include "qp_internal.h"
#include "qp_surface.h"
#include "sram.h"

// TODO: Optimize data representation
// Current format wastes 6 bits on each byte: | 0000 00BR | 0000 00BR | ...
// We could represent each pixel as a single bit when has_3color == false

// TODO: 0bpp surface? SRAM code uses 1bpp surfaces to store viewport
// however, the data on the arrays is not used for anything and could be
// skipped, checking dirtyness could be done by reading SRAM

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantum Painter API implementations

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Power control
bool qp_eink_panel_power(painter_device_t device, bool power_on) {
    struct painter_driver_t *                           driver = (struct painter_driver_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->driver_vtable;

    qp_comms_command(device, power_on ? vtable->opcodes.display_on : vtable->opcodes.display_off);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Screen clear
bool qp_eink_panel_clear(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct surface_painter_device_t *            black  = (struct surface_painter_device_t *)driver->black_surface;
    struct surface_painter_device_t *            red    = (struct surface_painter_device_t *)driver->red_surface;

    // Init the surfaces being wrapped, this will memset them to 0
    qp_init(driver->black_surface, driver->base.rotation);
    qp_init(driver->red_surface, driver->base.rotation);

    // memset 1s instead, if inverted format
    if (driver->invert_black) {
        memset(black->buffer, 0xFF, SURFACE_REQUIRED_BUFFER_BYTE_SIZE(driver->base.panel_width, driver->base.panel_height, driver->base.native_bits_per_pixel));
    }

    if (driver->invert_red) {
        memset(red->buffer, 0xFF, SURFACE_REQUIRED_BUFFER_BYTE_SIZE(driver->base.panel_width, driver->base.panel_height, driver->base.native_bits_per_pixel));
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reset can_flush flag back to true after timeout
uint32_t can_flush_callback(uint32_t trigger_time, void *cb_arg) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)cb_arg;

    driver->can_flush = true;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Screen flush
bool qp_eink_panel_flush(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *       driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->base.driver_vtable;
    struct surface_painter_device_t *                   black  = (struct surface_painter_device_t *)driver->black_surface;
    struct surface_painter_device_t *                   red    = (struct surface_painter_device_t *)driver->red_surface;

    if (!(black->is_dirty || red->is_dirty)) {
        qp_dprintf("qp_eink_panel_flush: done (no changes to be sent)\n");
        return true;
    }

    if (!driver->can_flush) {
        qp_dprintf("qp_eink_panel_flush: fail (can_flush == false)\n");
        return false;
    }

    if (driver->has_ram) {
        return sram_flush(device);
    }

    uint32_t n_bytes = driver->base.panel_width * driver->base.panel_height * driver->base.native_bits_per_pixel / 8;

    qp_comms_command(device, vtable->opcodes.send_black_data);
    qp_comms_send(device, black->buffer, n_bytes);

    // On B-W displays, we need to send empty red data, or we get noise
    qp_comms_command(device, vtable->opcodes.send_red_data);
    qp_comms_send(device, red->buffer, n_bytes);

    qp_comms_command(device, vtable->opcodes.refresh);

    // Set device on cant flush mode and schedule a function to reset the flag
    driver->can_flush = false;
    defer_exec(driver->timeout, can_flush_callback, (void *)device);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport to draw to
bool qp_eink_panel_viewport(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    qp_viewport(driver->black_surface, left, top, right, bottom);
    if (driver->has_3color) {
        qp_viewport(driver->red_surface, left, top, right, bottom);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stream pixel data to the current write position
bool qp_eink_panel_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct painter_driver_t *                    black  = (struct painter_driver_t *)driver->black_surface;
    uint8_t *                                    pixels = (uint8_t *)pixel_data;

    if (driver->has_ram) {
        return sram_pixdata(device, pixel_data, native_pixel_count);
    }

    // Since pixel data is stored as | 0000 00RB |, we have to append the pixels
    // one by one parsing those bits on each byte
    for (uint32_t i = 0; i < native_pixel_count; ++i) {
        // Calling driver function manually over using qp_pixdata to avoid getting
        // LOTS of qp_dprintf slowing it
        black->driver_vtable->pixdata(driver->black_surface, (const void *)(pixels[i] >> 0), 1);
        if (driver->has_3color) {
            struct painter_driver_t *red = (struct painter_driver_t *)driver->red_surface;
            red->driver_vtable->pixdata(driver->red_surface, (const void *)(pixels[i] >> 1), 1);
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert supplied palette entries into their native equivalents
void qp_eink_panel_palette_convert_bw(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    for (int16_t i = 0; i < palette_size; ++i) {
        HSV     hsv         = (HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v};
        uint8_t output      = (hsv.v > 127) ? 0 : 1;
        uint8_t invert_mask = (driver->invert_black << 0);

        palette[i].mono = output ^ invert_mask;
    }

    return true;
}

static inline uint32_t hsv_distance(uint8_t h, uint8_t s, uint8_t v, HSV hsv) {
    return (h - hsv.h) * (h - hsv.h) + (s - hsv.s) * (s - hsv.s) + (v - hsv.v) * (v - hsv.v);
}

bool qp_eink_panel_palette_convert_3c(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    for (int16_t i = 0; i < palette_size; ++i) {
        HSV      hsv            = (HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v};
        uint32_t black_distance = hsv_distance(HSV_BLACK, hsv);
        uint32_t red_distance   = hsv_distance(HSV_RED, hsv);
        uint32_t white_distance = hsv_distance(HSV_WHITE, hsv);

        // Default to white
        bool red   = false;
        bool black = false;

        uint32_t min_distance = QP_MIN(black_distance, QP_MIN(red_distance, white_distance));

        if (min_distance == black_distance)
            black = true;
        else if (min_distance == red_distance)
            red = true;

        uint8_t output      = (red << 1) | (black << 0);
        uint8_t invert_mask = (driver->invert_red << 1) | (driver->invert_black << 0);

        palette[i].mono = output ^ invert_mask;
    }

    return true;
}

bool qp_eink_panel_palette_convert(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    if (driver->has_3color) {
        return qp_eink_panel_palette_convert_3c(device, palette_size, palette);
    }

    return qp_eink_panel_palette_convert_bw(device, palette_size, palette);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Append pixels to the target location, keyed by the pixel index
bool qp_eink_panel_append_pixels(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    for (uint32_t i = 0; i < pixel_count; ++i) {
        target_buffer[pixel_offset + i] = palette[palette_indices[i]].mono;
    }

    return true;
}
