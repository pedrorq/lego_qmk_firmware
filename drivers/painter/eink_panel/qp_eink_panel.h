// Copyright 2023 Pablo Martinez (@elpekenin)
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

    // Whether or not the x/y coords should be swapped on 90/270 rotation
    bool swap_window_coords;

    // Opcodes for normal display operation
    // some may not exist on some displays
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

    // Information about display variants, allow functions to branch based on device specs
    bool has_3color;
    bool has_ram;

    // have to wait between flushes to avoid damaging the screen, time in ms
    uint32_t timeout;
    bool     can_flush;

    /** Information about the pixel format, default values (non-inverted, aka false) are
     *
     * Black bit: 0 for white / 1 for black
     * Red bit: 0 for white or black / 1 for red
     */
    bool invert_black;
    bool invert_red;


    // ---------------------------------------------------------
    // ----- Variant-specific config, some may not be used -----
    // ---------------------------------------------------------

    /** Virtual screens storing the 2 channels' data
     *
     * If display has builtin RAM, the surface objects are used to store the position in which to draw, thus
     * a 0bpp (no pixel data stored) surface is used, which can be referenced by both painter_device pointers
     *
     * Some screen controllers which support 3 colors may need receiving empty red-channel information to
     * work correctly in black/white variants, if so you'd need to create a second 1bpp surface which
     * will only be setup empty and get sent to the display when flushing, but will not be written at
     *
     * If your B/W display can work without that, you can make the red_surface a 0bpp buffer, which barely
     * consumes RAM
     */
    painter_device_t black_surface;
    painter_device_t red_surface;

    // Config to interact with display's builtin RAM
    struct {
        uint8_t write_status;
        uint8_t read_status;
        uint8_t write_data;
        uint8_t read_data;
        uint8_t sequential; // put SRAM on sequential mode
    } ram_opcodes;
    pin_t ram_chip_select_pin;

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

bool qp_eink_panel_palette_convert(painter_device_t device, int16_t palette_size, qp_pixel_t *palette);

bool qp_eink_panel_append_pixels(painter_device_t device, uint8_t *target_buffer, qp_pixel_t *palette, uint32_t pixel_offset, uint32_t pixel_count, uint8_t *palette_indices);
