// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "color.h"
#include "qp_internal.h"

#ifdef QUANTUM_PAINTER_SPI_ENABLE
#    include "qp_comms_spi.h"
#endif // QUANTUM_PAINTER_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers to create buffers with the apropiate size
#define EINK_BW_BYTES_REQD(w, h) (SURFACE_REQUIRED_BUFFER_BYTE_SIZE(w, h, 1))
#define EINK_3C_BYTES_REQD(w, h) (2 * EINK_BW_BYTES_REQD(w, h))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common TFT panel implementation using D/C, and RST pins.

// Driver vtable with extras
struct eink_panel_dc_reset_painter_driver_vtable_t {
    struct painter_driver_vtable_t base; // must be first, so it can be cast to/from the painter_driver_vtable_t* type

    // Number of bytes for transmitting x/y coordinates
    uint8_t num_window_bytes;

    // Whether or not the x/y coords should be swapped on 90/270 rotation
    bool swap_window_coords;

    // Opcodes for normal display operation
    // On/Off may not be needed, or even worse not exist on other displays
    struct {
        uint8_t display_on;
        uint8_t display_off;
        uint8_t send_black_data;
        uint8_t send_red_data;
        uint8_t refresh;
    } opcodes;
};

// Device definition
typedef struct eink_panel_dc_reset_painter_device_t {
    struct painter_driver_t base; // must be first, so it can be cast to/from the painter_device_t* type

    painter_device_t black_surface;
    painter_device_t red_surface;

    /** Information about the display
     *
     * TODO: not implemented yet
     * has_partial: whether the display supports partial refresing -- this driver wouldn't make much sense for them
     *
     * TODO: not implemented yet
     * has_ram: whether it has a built-in RAM in which to store the buffer, instead of using the MCU's -- this driver wouldn't make much sense for them
     *
     * has_3color: whether it's black/white or black/white/red
     */
    bool     has_partial;
    bool     has_ram;
    bool     has_3color;

    // have to wait between flushes to avoid damaging the screen, time in ms
    uint32_t timeout;
    bool     can_flush;

    /** Information about the pixel format used, default values (non-inverted, aka false) are
     *
     * Black bit: 0 for white / 1 for black
     * Red bit: 0 for white or black / 1 for red
     */
    bool     invert_black;
    bool     invert_red;

    union {
#ifdef QUANTUM_PAINTER_SPI_ENABLE
        // SPI-based configurables
        struct qp_comms_spi_dc_reset_config_t spi_dc_reset_config;
#endif // QUANTUM_PAINTER_SPI_ENABLE

        // TODO: I2C/parallel etc.
    };
} eink_panel_dc_reset_painter_device_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations for injecting into concrete driver vtables

bool qp_eink_panel_power(painter_device_t device, bool power_on);
bool qp_eink_panel_clear(painter_device_t device);
bool qp_eink_panel_flush(painter_device_t device);
bool qp_eink_panel_viewport(painter_device_t device, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
bool qp_eink_panel_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count);

bool qp_eink_panel_palette_convert_eink(painter_device_t device, int16_t palette_size, qp_pixel_t *palette);

bool qp_eink_panel_append_pixels_eink(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices);
