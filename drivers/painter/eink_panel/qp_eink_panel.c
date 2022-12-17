// Copyright 2022 Pablo Martinez (@elpekenin)
// Copyright 2021 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later

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
    // Flushing sends the framebuffer in RAM + refresh command to apply it
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->base.driver_vtable;

    if (driver->framebuffer == NULL) {
        qp_dprintf("Buffer is not a valid pointer, can't flush\n");
        return false;
    }

    uint32_t n_pixels = driver->base.panel_width * driver->base.panel_height / 8;

    qp_comms_command(device, vtable->opcodes.send_black_data);
    qp_comms_send(device, driver->framebuffer, n_pixels * driver->base.native_bits_per_pixel);

    if (vtable->has_3_colors) {
        qp_comms_command(device, vtable->opcodes.send_red_data);
        qp_comms_send(device, (driver->framebuffer)+n_pixels, n_pixels * driver->base.native_bits_per_pixel);
    }

    qp_comms_command(device, vtable->opcodes.refresh);
    return true;
}

// Viewport to draw to
bool qp_eink_panel_viewport(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
    // No-op, there's no viewport on this device
    return true;
}

// Stream pixel data to the current write position in GRAM
bool qp_eink_panel_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    // No-op, as there is no GRAM
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

        palette[i].eink3 = value;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Append pixels to the target location, keyed by the pixel index

bool qp_eink_panel_append_pixels_eink3(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    /* Buffer is twice the size of the screen,
     *   - 1st half: Black data
     *   - 2nd half: Red data
     */

    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    uint32_t total_offset, byte_offset;
    uint32_t red_offset = driver->base.panel_height * driver->base.panel_width / 8;
    uint8_t bit_offset, black_bit, red_bit;

    for (uint32_t i = 0; i < pixel_count; ++i) {
        // Get the bit representing each color and set it on the buffer
        total_offset = (pixel_offset + i);
        byte_offset = total_offset / 8;
        bit_offset  = 8 - (total_offset % 8);

        black_bit = palette[palette_indices[i]].eink3 & 0x01;
        red_bit   = palette[palette_indices[i]].eink3 & 0x02;

        if (black_bit)
            driver->framebuffer[byte_offset] |= 1 << bit_offset;
        else
            driver->framebuffer[byte_offset] &= ~(1 << bit_offset);

        // `red_offset` is based on panel size which is(or should be) a multiple of 8, hence previous offsets work
        if (red_bit)
            driver->framebuffer[byte_offset+red_offset] |= 1 << bit_offset;
        else
            driver->framebuffer[byte_offset+red_offset] &= ~(1 << bit_offset);
    }

    return true;
}

