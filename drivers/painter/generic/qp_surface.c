// Copyright 2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-2.0-or-later
#include "color.h"
#include "qp_surface.h"
#include "qp_draw.h"
#include "qp_comms_dummy.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common

struct surface_painter_driver_vtable_t {
    struct painter_driver_vtable_t base; // must be first, so it can be cast to/from the painter_driver_vtable_t* type

    bool (*target_pixdata_transfer)(struct painter_driver_t *surface_driver, struct painter_driver_t *target_driver, uint16_t x, uint16_t y, bool entire_surface);
};

// Driver storage
surface_painter_device_t surface_drivers[SURFACE_NUM_DEVICES] = {0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

static inline void increment_pixdata_location(surface_painter_device_t *surface) {
    // Increment the X-position
    surface->pixdata_x++;

    // If the x-coord has gone past the right-side edge, loop it back around and increment the y-coord
    if (surface->pixdata_x > surface->viewport_r) {
        surface->pixdata_x = surface->viewport_l;
        surface->pixdata_y++;
    }

    // If the y-coord has gone past the bottom, loop it back to the top
    if (surface->pixdata_y > surface->viewport_b) {
        surface->pixdata_y = surface->viewport_t;
    }
}

static void update_dirty(surface_painter_device_t *surface, uint16_t x, uint16_t y) {
    // Maintain dirty region
    if (surface->dirty_l > x) {
        surface->dirty_l = x;
    }
    if (surface->dirty_r < x) {
        surface->dirty_r = x;
    }
    if (surface->dirty_t > y) {
        surface->dirty_t = y;
    }
    if (surface->dirty_b < y) {
        surface->dirty_b = y;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable

static bool qp_surface_init(painter_device_t device, painter_rotation_t rotation) {
    struct painter_driver_t * driver  = (struct painter_driver_t *)device;
    surface_painter_device_t *surface = (surface_painter_device_t *)driver;
    memset(surface->buffer, 0, SURFACE_REQUIRED_BUFFER_BYTE_SIZE(driver->panel_width, driver->panel_height, driver->native_bits_per_pixel));
    surface->dirty_l  = 0;
    surface->dirty_t  = 0;
    surface->dirty_r  = surface->base.panel_width - 1;
    surface->dirty_b  = surface->base.panel_height - 1;
    surface->is_dirty = true;
    return true;
}

static bool qp_surface_power(painter_device_t device, bool power_on) {
    // No-op.
    return true;
}

static bool qp_surface_clear(painter_device_t device) {
    struct painter_driver_t *driver = (struct painter_driver_t *)device;
    driver->driver_vtable->init(device, driver->rotation); // Re-init the surface
    return true;
}

static bool qp_surface_flush(painter_device_t device) {
    struct painter_driver_t * driver  = (struct painter_driver_t *)device;
    surface_painter_device_t *surface = (surface_painter_device_t *)driver;
    surface->dirty_l = surface->dirty_t = UINT16_MAX;
    surface->dirty_r = surface->dirty_b = 0;
    surface->is_dirty                   = false;
    return true;
}

static bool qp_surface_viewport(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
    struct painter_driver_t * driver  = (struct painter_driver_t *)device;
    surface_painter_device_t *surface = (surface_painter_device_t *)driver;

    // Set the viewport locations
    surface->viewport_l = left;
    surface->viewport_t = top;
    surface->viewport_r = right;
    surface->viewport_b = bottom;

    // Reset the write location to the top left
    surface->pixdata_x = left;
    surface->pixdata_y = top;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable: rgb565

static inline void setpixel_rgb565(surface_painter_device_t *surface, uint16_t x, uint16_t y, uint16_t rgb565) {
    // Skip messing with the dirty info if the original value already matches
    if (surface->u16buffer[y * surface->base.panel_width + x] != rgb565) {
        // Update the dirty region
        update_dirty(surface, x, y);

        // Always dirty after a setpixel
        surface->is_dirty = true;

        // Update the pixel data in the buffer
        surface->u16buffer[y * surface->base.panel_width + x] = rgb565;
    }
}

static inline void append_pixel_rgb565(surface_painter_device_t *surface, uint16_t rgb565) {
    setpixel_rgb565(surface, surface->pixdata_x, surface->pixdata_y, rgb565);
    increment_pixdata_location(surface);
}

static inline void stream_pixdata_rgb565(surface_painter_device_t *surface, const uint16_t *data, uint32_t native_pixel_count) {
    for (uint32_t pixel_counter = 0; pixel_counter < native_pixel_count; ++pixel_counter) {
        append_pixel_rgb565(surface, data[pixel_counter]);
    }
}

// Stream pixel data to the current write position in GRAM
static bool qp_surface_pixdata_rgb565(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct painter_driver_t * driver  = (struct painter_driver_t *)device;
    surface_painter_device_t *surface = (surface_painter_device_t *)driver;
    stream_pixdata_rgb565(surface, (const uint16_t *)pixel_data, native_pixel_count);
    return true;
}

// Pixel colour conversion
static bool qp_surface_palette_convert_rgb565_swapped(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    for (int16_t i = 0; i < palette_size; ++i) {
        RGB      rgb      = hsv_to_rgb_nocie((HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v});
        uint16_t rgb565   = (((uint16_t)rgb.r) >> 3) << 11 | (((uint16_t)rgb.g) >> 2) << 5 | (((uint16_t)rgb.b) >> 3);
        palette[i].rgb565 = __builtin_bswap16(rgb565);
    }
    return true;
}

// Append pixels to the target location, keyed by the pixel index
static bool qp_surface_append_pixels_rgb565(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    uint16_t *buf = (uint16_t *)target_buffer;
    for (uint32_t i = 0; i < pixel_count; ++i) {
        buf[pixel_offset + i] = palette[palette_indices[i]].rgb565;
    }
    return true;
}

static bool rgb565_target_pixdata_transfer(struct painter_driver_t *surface_driver, struct painter_driver_t *target_driver, uint16_t x, uint16_t y, bool entire_surface) {
    surface_painter_device_t *surface_handle = (surface_painter_device_t *)surface_driver;

    uint16_t l = entire_surface ? 0 : surface_handle->dirty_l;
    uint16_t t = entire_surface ? 0 : surface_handle->dirty_t;
    uint16_t r = entire_surface ? (surface_handle->base.panel_width - 1) : surface_handle->dirty_r;
    uint16_t b = entire_surface ? (surface_handle->base.panel_height - 1) : surface_handle->dirty_b;

    // Set the target drawing area
    bool ok = qp_viewport((painter_device_t)target_driver, x + l, y + t, x + r, y + b);
    if (!ok) {
        qp_dprintf("rgb565_target_pixdata_transfer: fail (could not set target viewport)\n");
        return false;
    }

    // Housekeeping of the amount of pixels to transfer
    uint32_t  total_pixel_count = (8 * QUANTUM_PAINTER_PIXDATA_BUFFER_SIZE) / surface_driver->native_bits_per_pixel;
    uint32_t  pixel_counter     = 0;
    uint16_t *target_buffer     = (uint16_t *)qp_internal_global_pixdata_buffer;

    // Fill the global pixdata area so that we can start transferring to the panel
    for (uint16_t y = t; y <= b; ++y) {
        for (uint16_t x = l; x <= r; ++x) {
            // Update the target buffer
            target_buffer[pixel_counter++] = surface_handle->u16buffer[y * surface_handle->base.panel_width + x];

            // If we've accumulated enough data, send it
            if (pixel_counter == total_pixel_count) {
                ok = qp_pixdata((painter_device_t)target_driver, qp_internal_global_pixdata_buffer, pixel_counter);
                if (!ok) {
                    qp_dprintf("rgb565_target_pixdata_transfer: fail (could not stream pixdata to target)\n");
                    return false;
                }
                // Reset the counter
                pixel_counter = 0;
            }
        }
    }

    // If there's any leftover data, send it
    if (pixel_counter > 0) {
        ok = qp_pixdata((painter_device_t)target_driver, qp_internal_global_pixdata_buffer, pixel_counter);
        if (!ok) {
            qp_dprintf("rgb565_target_pixdata_transfer: fail (could not stream pixdata to target)\n");
            return false;
        }
    }

    return true;
}

const struct surface_painter_driver_vtable_t rgb565_surface_driver_vtable = {
    .base =
        {
            .init            = qp_surface_init,
            .power           = qp_surface_power,
            .clear           = qp_surface_clear,
            .flush           = qp_surface_flush,
            .pixdata         = qp_surface_pixdata_rgb565,
            .viewport        = qp_surface_viewport,
            .palette_convert = qp_surface_palette_convert_rgb565_swapped,
            .append_pixels   = qp_surface_append_pixels_rgb565,
        },
    .target_pixdata_transfer = rgb565_target_pixdata_transfer,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable: mono1bpp
#define QP_SWAP(a, b) {   \
    uint16_t temp = a; \
    a = b;             \
    b = temp;          \
}

static inline void setpixel_mono1bpp(surface_painter_device_t *surface, uint16_t x, uint16_t y, bool mono_pixel) {
    // Skip messing with the dirty info if the original value already matches

    uint16_t width  = surface->base.panel_width;
    uint16_t height = surface->base.panel_height;

    switch (surface->base.rotation) {
        case QP_ROTATION_0:
            break;

        case QP_ROTATION_90:
            QP_SWAP(x, y);
            x = width - x -1;
            break;

        case QP_ROTATION_180:
            x = width  - x - 1;
            y = height - y - 1;
            break;

        case QP_ROTATION_270:
            QP_SWAP(x, y);
            y = height - y - 1;
            break;
    }

    uint32_t pixel_num   = x + y * width;
    surface->index       = pixel_num;
    uint32_t byte_offset = pixel_num / 8;
    uint8_t  bit_offset  = 7 - pixel_num % 8;
    bool     curr_val    = (surface->u8buffer[byte_offset] & (1 << bit_offset)) ? true : false;

    if (curr_val != mono_pixel) {
        // Update the dirty region
        update_dirty(surface, x, y);

        // Always dirty after a setpixel
        surface->is_dirty = true;

        // Update the pixel data in the buffer
        if (mono_pixel) {
            surface->u8buffer[byte_offset] |= (1 << bit_offset);
        } else {
            surface->u8buffer[byte_offset] &= ~(1 << bit_offset);
        }
    }
}

static inline void append_pixel_mono1bpp(surface_painter_device_t *surface, bool mono_pixel) {
    setpixel_mono1bpp(surface, surface->pixdata_x, surface->pixdata_y, mono_pixel);
    increment_pixdata_location(surface);
}

static inline void stream_pixdata_mono1bpp(surface_painter_device_t *surface, const uint8_t *data, uint32_t native_pixel_count) {
    for (uint32_t pixel_counter = 0; pixel_counter < native_pixel_count; ++pixel_counter) {
        uint32_t byte_offset = pixel_counter / 8;
        uint8_t  bit_offset  = pixel_counter % 8;
        append_pixel_mono1bpp(surface, (data[byte_offset] & (1 << bit_offset)) ? true : false);
    }
}

// Stream pixel data to the current write position in GRAM
static bool qp_surface_pixdata_mono1bpp(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct painter_driver_t * driver  = (struct painter_driver_t *)device;
    surface_painter_device_t *surface = (surface_painter_device_t *)driver;
    stream_pixdata_mono1bpp(surface, (const uint8_t *)pixel_data, native_pixel_count);
    return true;
}

// Pixel colour conversion
static bool qp_surface_palette_convert_mono1bpp(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    for (int16_t i = 0; i < palette_size; ++i) {
        palette[i].mono = (palette[i].hsv888.v > 127) ? 1 : 0;
    }
    return true;
}

// Append pixels to the target location, keyed by the pixel index
static bool qp_surface_append_pixels_mono1bpp(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    for (uint32_t i = 0; i < pixel_count; ++i) {
        uint32_t pixel_num   = pixel_offset + i;
        uint32_t byte_offset = pixel_num / 8;
        uint8_t  bit_offset  = 7 - pixel_num % 8;
        if (palette[palette_indices[i]].mono) {
            target_buffer[byte_offset] |= (1 << bit_offset);
        } else {
            target_buffer[byte_offset] &= ~(1 << bit_offset);
        }
    }
    return true;
}

static bool mono1bpp_target_pixdata_transfer(struct painter_driver_t *surface_driver, struct painter_driver_t *target_driver, uint16_t x, uint16_t y, bool entire_surface) {
    surface_painter_device_t *surface_handle = (surface_painter_device_t *)surface_driver;

    uint16_t l = entire_surface ? 0 : surface_handle->dirty_l;
    uint16_t t = entire_surface ? 0 : surface_handle->dirty_t;
    uint16_t r = entire_surface ? (surface_handle->base.panel_width - 1) : surface_handle->dirty_r;
    uint16_t b = entire_surface ? (surface_handle->base.panel_height - 1) : surface_handle->dirty_b;

    // Move to byte-wise boundaries -- no need to modify the top/bottom, as these aren't byte-wise constrained
    uint16_t panel_l = x + l;
    uint16_t panel_t = y + t;
    uint16_t panel_r = x + r;
    uint16_t panel_b = y + b;
    panel_l          = (panel_l / 8) * 8;
    panel_r          = ((((panel_r + 1) + 7) / 8) * 8) - 1;

    // Set the target drawing area
    bool ok = qp_viewport((painter_device_t)target_driver, panel_l, panel_t, panel_r, panel_b);
    if (!ok) {
        qp_dprintf("mono1bpp_target_pixdata_transfer: fail (could not set target viewport)\n");
        return false;
    }

    // Housekeeping of the amount of pixels to transfer
    uint32_t total_pixel_count = (8 * QUANTUM_PAINTER_PIXDATA_BUFFER_SIZE) / surface_driver->native_bits_per_pixel;
    uint32_t pixel_counter     = 0;

    // Fill the global pixdata area so that we can start transferring to the panel
    for (uint16_t y = panel_t; y <= panel_b; ++y) {
        for (uint16_t x = panel_l; x <= panel_r; x += 8) {
            // Update the target buffer
            qp_internal_global_pixdata_buffer[pixel_counter / 8] = surface_handle->u8buffer[(y * surface_handle->base.panel_width + x) / 8];
            pixel_counter += 8;

            // If we've accumulated enough data, send it
            if (pixel_counter >= total_pixel_count) {
                ok = qp_pixdata((painter_device_t)target_driver, qp_internal_global_pixdata_buffer, pixel_counter);
                if (!ok) {
                    qp_dprintf("mono1bpp_target_pixdata_transfer: fail (could not stream pixdata to target)\n");
                    return false;
                }
                // Reset the counter
                pixel_counter = 0;
            }
        }
    }

    // If there's any leftover data, send it
    if (pixel_counter > 0) {
        ok = qp_pixdata((painter_device_t)target_driver, qp_internal_global_pixdata_buffer, pixel_counter);
        if (!ok) {
            qp_dprintf("mono1bpp_target_pixdata_transfer: fail (could not stream pixdata to target)\n");
            return false;
        }
    }

    return true;
}

const struct surface_painter_driver_vtable_t mono1bpp_surface_driver_vtable = {
    .base =
        {
            .init            = qp_surface_init,
            .power           = qp_surface_power,
            .clear           = qp_surface_clear,
            .flush           = qp_surface_flush,
            .pixdata         = qp_surface_pixdata_mono1bpp,
            .viewport        = qp_surface_viewport,
            .palette_convert = qp_surface_palette_convert_mono1bpp,
            .append_pixels   = qp_surface_append_pixels_mono1bpp,
        },
    .target_pixdata_transfer = mono1bpp_target_pixdata_transfer,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable: 0bpp -- doesnt store data, just pixel location

// Streaming a pixel just increments the pixdata location
static bool qp_surface_pixdata_0bpp(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct painter_driver_t  *driver  = (struct painter_driver_t *)device;
    surface_painter_device_t *surface = (surface_painter_device_t *)driver;
    increment_pixdata_location(surface);
    return true;
}

static bool qp_surface_palette_convert_0bpp(painter_device_t device, int16_t palette_size, qp_pixel_t *palette) {
    // No-op.
    return true;
}

static bool qp_surface_append_pixels_0bpp(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices) {
    // No-op.
    return true;
}

const struct painter_driver_vtable_t empty_0bpp_surface_driver_vtable = {
    .init            = qp_surface_init,
    .power           = qp_surface_power,
    .clear           = qp_surface_clear,
    .flush           = qp_surface_flush,
    .pixdata         = qp_surface_pixdata_0bpp,
    .viewport        = qp_surface_viewport,
    .palette_convert = qp_surface_palette_convert_0bpp,
    .append_pixels   = qp_surface_append_pixels_0bpp,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Factory functions for creating a handle to a surface

#define SURFACE_FACTORY_FUNCTION_IMPL(function_name, vtable, bpp)                                                                                                             \
    painter_device_t(function_name##_advanced)(surface_painter_device_t * device_table, size_t device_table_len, uint16_t panel_width, uint16_t panel_height, void *buffer) { \
        for (uint32_t i = 0; i < device_table_len; ++i) {                                                                                                                     \
            surface_painter_device_t *driver = &device_table[i];                                                                                                              \
            if (!driver->base.driver_vtable) {                                                                                                                                \
                driver->base.driver_vtable         = (struct painter_driver_vtable_t *)&(vtable);                                                                             \
                driver->base.native_bits_per_pixel = (bpp);                                                                                                                   \
                driver->base.comms_vtable          = &dummy_comms_vtable;                                                                                                     \
                driver->base.panel_width           = panel_width;                                                                                                             \
                driver->base.panel_height          = panel_height;                                                                                                            \
                driver->base.rotation              = QP_ROTATION_0;                                                                                                           \
                driver->base.offset_x              = 0;                                                                                                                       \
                driver->base.offset_y              = 0;                                                                                                                       \
                driver->buffer                     = buffer;                                                                                                                  \
                return (painter_device_t)driver;                                                                                                                              \
            }                                                                                                                                                                 \
        }                                                                                                                                                                     \
        return NULL;                                                                                                                                                          \
    }                                                                                                                                                                         \
    painter_device_t(function_name)(uint16_t panel_width, uint16_t panel_height, void *buffer) {                                                                              \
        return (function_name##_advanced)(surface_drivers, SURFACE_NUM_DEVICES, panel_width, panel_height, buffer);                                                           \
    }

SURFACE_FACTORY_FUNCTION_IMPL(qp_make_rgb565_surface, rgb565_surface_driver_vtable, 16);
SURFACE_FACTORY_FUNCTION_IMPL(qp_make_mono1bpp_surface, mono1bpp_surface_driver_vtable, 1);
SURFACE_FACTORY_FUNCTION_IMPL(qp_make_0bpp_surface, empty_0bpp_surface_driver_vtable, 0);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Drawing routine to copy out the dirty region and send it to another device

bool qp_surface_draw(painter_device_t surface, painter_device_t target, uint16_t x, uint16_t y, bool entire_surface) {
    struct painter_driver_t * surface_driver = (struct painter_driver_t *)surface;
    surface_painter_device_t *surface_handle = (surface_painter_device_t *)surface_driver;
    struct painter_driver_t * target_driver  = (struct painter_driver_t *)target;

    // If we're not dirty... we're done.
    if (!surface_handle->is_dirty) {
        qp_dprintf("qp_surface_draw: ok (not dirty, skipping)\n");
        return true;
    }

    // If we have incompatible bit depths, drop out
    if (surface_driver->native_bits_per_pixel != target_driver->native_bits_per_pixel) {
        qp_dprintf("qp_surface_draw: fail (incompatible bpp: surface=%d, target=%d)\n", (int)surface_driver->native_bits_per_pixel, (int)target_driver->native_bits_per_pixel);
        return false;
    }

    // Offload to the pixdata transfer function
    struct surface_painter_driver_vtable_t *vtbl = (struct surface_painter_driver_vtable_t *)surface_driver->driver_vtable;
    bool                                    ok   = vtbl->target_pixdata_transfer(surface_driver, target_driver, x, y, entire_surface);
    if (!ok) {
        qp_dprintf("qp_surface_draw: fail (could not transfer pixel data)\n");
        return false;
    }

    // Clear the dirty info for the surface
    ok = qp_flush(surface);
    if (!ok) {
        qp_dprintf("qp_surface_draw: fail (could not flush)\n");
        return false;
    }
    qp_dprintf("qp_surface_draw: ok\n");
    return true;
}
