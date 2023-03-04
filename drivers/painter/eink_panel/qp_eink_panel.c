// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_comms.h"
#include "qp_draw.h"
#include "qp_eink_panel.h"
#include "qp_internal.h"
#include "qp_surface.h"
#include "qp_surface_internal.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantum Painter API implementations

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Power control
bool qp_eink_panel_power(painter_device_t device, bool power_on) {
    painter_driver_t *                           driver = (painter_driver_t *)device;
    eink_panel_dc_reset_painter_driver_vtable_t *vtable = (eink_panel_dc_reset_painter_driver_vtable_t *)driver->driver_vtable;

    qp_comms_command(device, power_on ? vtable->opcodes.display_on : vtable->opcodes.display_off);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Screen clear
bool qp_eink_panel_clear(painter_device_t device) {
    // this function gets called by eink's init
    // we init red even if display doesn't support 3 color, empty data for it may be needed
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;
    surface_painter_device_t *            black  = (surface_painter_device_t *)driver->black_surface;
    surface_painter_device_t *            red    = (surface_painter_device_t *)driver->red_surface;

    qp_init(driver->black_surface, driver->base.rotation);
    if (driver->invert_black) {
        memset(black->buffer, 0xFF, SURFACE_REQUIRED_BUFFER_BYTE_SIZE(driver->base.panel_width, driver->base.panel_height, black->base.native_bits_per_pixel));
    }

    qp_init(driver->red_surface, driver->base.rotation);
    if (driver->invert_red) {
        memset(red->buffer, 0xFF, SURFACE_REQUIRED_BUFFER_BYTE_SIZE(driver->base.panel_width, driver->base.panel_height, red->base.native_bits_per_pixel));
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reset can_flush flag back to true after timeout
uint32_t can_flush_callback(uint32_t trigger_time, void *cb_arg) {
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)cb_arg;

    driver->can_flush = true;

    return 0;
}

// Set can_flush to false and schedule its cleanup
void qp_eink_update_can_flush(painter_device_t device) {
    eink_panel_dc_reset_painter_device_t *driver  = (eink_panel_dc_reset_painter_device_t *)device;

    driver->can_flush = false;
    defer_exec(driver->timeout, can_flush_callback, (void *)device);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Screen flush
bool qp_eink_panel_flush(painter_device_t device) {
    eink_panel_dc_reset_painter_device_t *       driver  = (eink_panel_dc_reset_painter_device_t *)device;
    eink_panel_dc_reset_painter_driver_vtable_t *vtable  = (eink_panel_dc_reset_painter_driver_vtable_t *)driver->base.driver_vtable;
    surface_painter_device_t *                   black   = (surface_painter_device_t *)driver->black_surface;
    surface_painter_device_t *                   red     = (surface_painter_device_t *)driver->red_surface;
    uint32_t                                     n_bytes = EINK_BW_BYTES_REQD(driver->base.panel_width, driver->base.panel_height);

    if (!(black->dirty.is_dirty || red->dirty.is_dirty)) {
        qp_dprintf("qp_eink_panel_flush: done (no changes to be sent)\n");
        return true;
    }

    if (!driver->can_flush) {
        qp_dprintf("qp_eink_panel_flush: fail (can_flush == false)\n");
        return false;
    }

    qp_comms_command(device, vtable->opcodes.send_black_data);
    qp_comms_send(device, black->buffer, n_bytes);

    // even if we have a B/W display, ot may need to receive empty red data
    // to prevent noisy drawing, we check whether this is needed by looking
    // at bits_per_pixel, should be 0 if we don't need to send and 1 if we do
    if (red->base.native_bits_per_pixel) {
        qp_comms_command(device, vtable->opcodes.send_red_data);
        qp_comms_send(device, red->buffer, n_bytes);
    }

    qp_comms_command(device, vtable->opcodes.refresh);

    qp_eink_update_can_flush(device);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport to draw to
bool qp_eink_panel_viewport(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;

    qp_viewport(driver->black_surface, left, top, right, bottom);
    if (driver->has_3color)
        qp_viewport(driver->red_surface, left, top, right, bottom);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stream pixel data to the current write position
static inline void decode_bitmask(uint8_t *pixels, uint32_t byte, uint8_t *black, uint8_t *red) {
    /* Convert pixel data into convenient representation
     *
     * B1 R1  B2 R2  B3 R3  B4 R4 || B5 R5  B6 R6  B7 R7  B8 R8
     * Becomes
     * black_data: B8 B7 B6 B5 B4 B3 B2 B1
     * red_data:   R8 R7 R6 R5 R4 R3 R2 R1
     *
     * Note: Will be grabbing 8 pixels at most, thus uint16_t is enough
     * Note: Always accessing `byte + 1` might go out of the buffer if display's (w*h) % 8 != 0
     */
    uint16_t raw_data = (pixels[byte] << 8) | (pixels[byte + 1]);

    // clear data so we can simply |=
    *black = 0;
    *red   = 0;

    uint16_t black_mask = 1 << 15;
    uint16_t red_mask   = 1 << 14;

    for (uint8_t i = 0; i < 8; ++i) {
        bool black_bit = raw_data & black_mask;
        bool red_bit   = raw_data & red_mask;

        black_mask >>= 2;
        red_mask   >>= 2;

        *black |= (black_bit << i);
        *red   |= (red_bit   << i);
    }
}

bool qp_eink_panel_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;
    painter_driver_t *                    black  = (painter_driver_t *)driver->black_surface;
    painter_driver_t *                    red    = (painter_driver_t *)driver->red_surface;
    uint8_t *                             pixels = (uint8_t *)pixel_data;

    /* By parsing 2 bytes at a time, we can make "buffers" of 8 pixels
     * We do this &'ing and and shifting each bit to its position
     *
     *
     * - 2 bytes (8 pixels) => Increse i by that amount
     * - We don't have to compute a bit_offset on the loop's indexing
     *   Will always land at the start of a byte
     * - Since each byte stores 4 pixels, byte offset is i / 4
     * - We don't want to grab data further down on the buffer
     *   have to check if current index + 8 goes out of bounds
     */
    uint32_t i = 0;
    uint8_t black_data, red_data;
    while (i < native_pixel_count) {
        // at most, 8 pixels per cycle
        uint8_t pixels_this_loop = QP_MIN(native_pixel_count - i, 8);
        uint8_t byte = i / 4;

        // stream data to display
        decode_bitmask(pixels, byte, &black_data, &red_data);
        black->driver_vtable->pixdata(driver->black_surface, (const void *)&black_data, pixels_this_loop);
        if (driver->has_3color)
            red->driver_vtable->pixdata(driver->red_surface, (const void *)&red_data, pixels_this_loop);

        // update position
        i += pixels_this_loop;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert supplied palette entries into their native equivalents
bool qp_eink_panel_palette_convert_bw(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;

    for (int16_t i = 0; i < palette_size; ++i) {
        HSV     hsv     = (HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v};
        uint8_t output  = (hsv.v > 127) ? 0 : 1;
        palette[i].mono = output ^ driver->invert_black;
    }

    return true;
}

static inline uint32_t hsv_distance(uint8_t h, uint8_t s, uint8_t v, HSV hsv) {
    return (h - hsv.h) * (h - hsv.h) + (s - hsv.s) * (s - hsv.s) + (v - hsv.v) * (v - hsv.v);
}

bool qp_eink_panel_palette_convert_3c(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;

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
        uint8_t invert_mask = (driver->invert_red << 1) | driver->invert_black;

        palette[i].mono = output ^ invert_mask;
    }

    return true;
}

bool qp_eink_panel_palette_convert(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;

    if (driver->has_3color) {
        return qp_eink_panel_palette_convert_3c(device, palette_size, palette);
    }

    return qp_eink_panel_palette_convert_bw(device, palette_size, palette);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Append pixels to the target location, keyed by the pixel index
bool qp_eink_panel_append_pixels(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    // data should end up arranged:
    // B1R1B2R2  B3R3B4R4 | ...
    for (uint32_t i = 0; i < pixel_count; ++i) {
        uint32_t pixel_num   = pixel_offset + i;

        // each pixel takes 2 bits, aka each byte holds 4 pixels, offset based on that
        uint32_t byte_offset = pixel_num / 4;
        uint8_t  bit_offset  = 3 - (pixel_num % 4);

        bool black_bit = palette[palette_indices[i]].mono & 1;
        bool red_bit   = palette[palette_indices[i]].mono & 2;

        uint8_t black_mask = 1 << (2 * bit_offset + 1);
        uint8_t red_mask   = 1 << (2 * bit_offset + 0);

        if (black_bit)
            target_buffer[byte_offset] |= black_mask;
        else
            target_buffer[byte_offset] &= ~black_mask;

        if (red_bit)
            target_buffer[byte_offset] |= red_mask;
        else
            target_buffer[byte_offset] &= ~red_mask;
    }

    return true;
}

bool qp_eink_panel_append_pixdata(painter_device_t device, uint8_t *target_buffer, uint32_t pixdata_offset, uint8_t pixdata_byte) {
    // e-Ink should not receive a >8 bpp image
    return false;
}
