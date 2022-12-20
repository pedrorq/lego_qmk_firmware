// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_internal.h"
#include "qp_comms.h"
#include "qp_draw.h"
#include "qp_eink_panel.h"
#include "qp_surface.h"

// TODO: Add rotation support
// TODO: Add support for screens with partial refresh
// TODO: Add support for displays with builtin RAM

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantum Painter API implementations

// Power control
bool qp_eink_panel_power(painter_device_t device, bool power_on) {
    struct painter_driver_t *                           driver = (struct painter_driver_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->driver_vtable;

    qp_comms_command(device, power_on ? vtable->opcodes.display_on : vtable->opcodes.display_off);

    return true;
}

// Screen clear
bool qp_eink_panel_clear(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    qp_rect(driver->black_surface, 0, 0, driver->base.panel_width, driver->base.panel_height, HSV_WHITE, true);
    qp_rect(driver->red_surface,   0, 0, driver->base.panel_width, driver->base.panel_height, HSV_WHITE, true);

    return true;
}

// Screen flush
bool qp_eink_panel_flush(painter_device_t device) {
    // Flushing sends the framebuffer in RAM + refresh command to apply it
    struct eink_panel_dc_reset_painter_device_t        *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    // struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->base.driver_vtable;
    struct surface_painter_device_t                    *black  = (struct surface_painter_device_t *)driver->black_surface;
    struct surface_painter_device_t                    *red    = (struct surface_painter_device_t *)driver->red_surface;

    // qp_comms_command(device, vtable->opcodes.send_black_data);
    // qp_comms_send(device, black->u8buffer, black->base.panel_width * black->base.panel_height / 8);

    // qp_comms_command(device, vtable->opcodes.send_red_data);
    // qp_comms_send(device, red->u8buffer, red->base.panel_width * red->base.panel_height / 8);

    // qp_comms_command(device, vtable->opcodes.refresh);

    for (uint32_t i=0; i < driver->base.panel_height * driver->base.panel_width / 8; ++i) {
        qp_dprintf(
            "%d %d  %d %d  %d %d  %d %d  %d %d  %d %d  %d %d  %d %d  ",
            (black->u8buffer[i] >> 0) & 1, (red->u8buffer[i] >> 0) & 1,
            (black->u8buffer[i] >> 1) & 1, (red->u8buffer[i] >> 1) & 1,
            (black->u8buffer[i] >> 2) & 1, (red->u8buffer[i] >> 2) & 1,
            (black->u8buffer[i] >> 3) & 1, (red->u8buffer[i] >> 3) & 1,
            (black->u8buffer[i] >> 4) & 1, (red->u8buffer[i] >> 4) & 1,
            (black->u8buffer[i] >> 5) & 1, (red->u8buffer[i] >> 5) & 1,
            (black->u8buffer[i] >> 6) & 1, (red->u8buffer[i] >> 6) & 1,
            (black->u8buffer[i] >> 7) & 1, (red->u8buffer[i] >> 7) & 1
        );
    }
    qp_dprintf("\n");

    return true;
}

// Viewport to draw to
bool qp_eink_panel_viewport(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    qp_viewport(driver->black_surface, left, top, right, bottom);
    qp_viewport(driver->red_surface,   left, top, right, bottom);

    return true;
}

// Stream pixel data to the current write position in GRAM
bool qp_eink_panel_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct painter_driver_t                     *black  = (struct painter_driver_t *)driver->black_surface;
    struct painter_driver_t                     *red    = (struct painter_driver_t *)driver->red_surface;

    qp_pixdata((void *) black, pixel_data, native_pixel_count);
    qp_pixdata((void *) red, pixel_data+native_pixel_count, native_pixel_count);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert supplied palette entries into their native equivalents

uint16_t hsv_distance(uint8_t h, uint8_t s, uint8_t v, HSV hsv) {
    return (h-hsv.h)*(h-hsv.h) + (s-hsv.s)*(s-hsv.s) + (v-hsv.v)*(v-hsv.v);
}

bool qp_eink_panel_palette_convert_eink3(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    for (int16_t i = 0; i < palette_size; ++i) {
        HSV hsv = (HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v};
        uint16_t black_distance = hsv_distance(HSV_BLACK, hsv);
        uint16_t red_distance   = hsv_distance(HSV_RED, hsv);
        uint16_t white_distance = hsv_distance(HSV_WHITE, hsv);

        uint8_t value;
        uint16_t min_distance = QP_MIN(black_distance, QP_MIN(red_distance, white_distance));
        if (min_distance == black_distance)
            value = 0b01;

        else if (min_distance == red_distance)
            value = 0b10;

        else
            value = 0b00;

        palette[i].mono = value;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Append pixels to the target location, keyed by the pixel index

bool qp_eink_panel_append_pixels_eink3(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    // Black data
    for (uint32_t i = 0; i < pixel_count; ++i) {
        uint32_t pixel_num   = pixel_offset + i;
        uint32_t byte_offset = pixel_num / 8;
        uint8_t  bit_offset  = pixel_num % 8;
        if (palette[palette_indices[i]].mono & 1) {
            target_buffer[byte_offset] |= (1 << bit_offset);
        } else {
            target_buffer[byte_offset] &= ~(1 << bit_offset);
        }
    }

    // Red data
    for (uint32_t i = 0; i < pixel_count; ++i) {
        uint32_t pixel_num   = pixel_offset + pixel_count + i;
        uint32_t byte_offset = pixel_num / 8;
        uint8_t  bit_offset  = pixel_num % 8;
        if (palette[palette_indices[i]].mono & 2) {
            target_buffer[byte_offset] |= (1 << bit_offset);
        } else {
            target_buffer[byte_offset] &= ~(1 << bit_offset);
        }
    }

    return true;
}

