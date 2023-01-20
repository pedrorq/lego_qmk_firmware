// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_internal.h"
#include "qp_comms.h"
#include "qp_il91874.h"
#include "qp_il91874_opcodes.h"
#include "qp_eink_panel.h"
#include "qp_surface.h"
#include "sram.h"

#ifdef QUANTUM_PAINTER_IL91874_SPI_ENABLE
#    include <qp_comms_spi.h>
#endif // QUANTUM_PAINTER_IL91874_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common

// Driver storage
eink_panel_dc_reset_painter_device_t il91874_drivers[IL91874_NUM_DEVICES] = {0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization

bool qp_il91874_init(painter_device_t device, painter_rotation_t rotation) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    uint8_t width_lsb  = (driver->base.panel_width) & 0xFF;
    uint8_t width_msb  = (driver->base.panel_width >> 8) & 0xFF;
    uint8_t height_lsb = (driver->base.panel_height) & 0xFF;
    uint8_t height_msb = (driver->base.panel_height >> 8) & 0xFF;

    // TODO: Add config changes for BW variant (if needed), this is 3color init

    // clang-format off
    const uint8_t il91874_init_sequence[] = {
        // Command,                 Delay,  N, Data[N]
        IL91874_POWER_ON,             120,  0,
        IL91874_PANEL_SETTING,          0,  1, 0xAF, // 0x1F
        IL91874_PLL,                    0,  1, 0x3A,
        IL91874_POWER_SETTING,          0,  5, 0x03, 0x00, 0x2B, 0x2B, 0x09,
        IL91874_BOOSTER_SOFT_START,     0,  3, 0x07, 0x07, 0x17,
        IL91874_MISTERY_COMMAND,        0,  2, 0x60, 0xA5,
        IL91874_MISTERY_COMMAND,        0,  2, 0x89, 0xA5,
        IL91874_MISTERY_COMMAND,        0,  2, 0x90, 0x00,
        IL91874_MISTERY_COMMAND,        0,  2, 0x93, 0xAD,
        IL91874_MISTERY_COMMAND,        0,  2, 0x73, 0x41,
        IL91874_VCM_DC_SETTING,         0,  1, 0x12,
        IL91874_CDI,                    0,  1, 0x87,
        IL91874_LUT1,                   0, 44, 0x0, 0x0, 0x0, 0x1A, 0x1A, 0x0, 0x0, 0x01, 0x0, 0xA, 0xA, 0x0, 0x0, 0x8, 0x0, 0xE, 0x1, 0xE, 0x1, 0x10, 0x0, 0xA, 0xA, 0x0, 0x0, 0x8, 0x0, 0x4, 0x10, 0x0, 0x0, 0x5, 0x0, 0x3, 0xE, 0x0, 0x0, 0xA, 0x0, 0x23, 0x0, 0x0, 0x0, 0x1,
        IL91874_LUTWW,                  0, 42, 0x90, 0x1A, 0x1A, 0x0, 0x0, 0x1, 0x40, 0x0A, 0x0A, 0x0, 0x0, 0x8, 0x84, 0xE, 0x1, 0xE, 0x1, 0x10, 0x80, 0xA, 0xA, 0x0, 0x0, 0x8, 0x0, 0x4, 0x10, 0x0, 0x0, 0x5, 0x0, 0x3, 0xE, 0x0, 0x0, 0xA, 0x0, 0x23, 0x0, 0x0, 0x0, 0x1,
        IL91874_LUTBW,                  0, 42, 0xA0, 0x1A, 0x1A, 0x0, 0x0, 0x1, 0x0, 0xA, 0xA, 0x0, 0x0, 0x8, 0x84, 0xE, 0x1, 0xE, 0x1, 0x10, 0x90, 0xA, 0xA, 0x0, 0x0, 0x8, 0xB0, 0x4, 0x10, 0x0, 0x0, 0x5, 0xB0, 0x3, 0xE, 0x0, 0x0, 0xA, 0xC0, 0x23, 0x0, 0x0, 0x0, 0x1,
        IL91874_LUTWB,                  0, 42, 0x90, 0x1A, 0x1A, 0x0, 0x0, 0x1, 0x40, 0xA, 0xA, 0x0, 0x0, 0x8, 0x84, 0xE, 0x1, 0xE, 0x1, 0x10, 0x80, 0xA, 0xA, 0x0, 0x0, 0x8, 0x0, 0x4, 0x10, 0x0, 0x0, 0x5, 0x0, 0x3, 0xE, 0x0, 0x0, 0xA, 0x0, 0x23, 0x0, 0x0, 0x0, 0x1,
        IL91874_LUTBB,                  0, 42, 0x90, 0x1a, 0x1A, 0x0, 0x0, 0x1, 0x20, 0xA, 0xA, 0x0, 0x0, 0x8, 0x84, 0xE, 0x1, 0xE, 0x1, 0x10, 0x10, 0xA, 0xA, 0x0, 0x0, 0x8, 0x0, 0x4, 0x10, 0x0, 0x0, 0x5, 0x0, 0x3, 0xE, 0x0, 0x0, 0xA, 0x0, 0x23, 0x0, 0x0, 0x0, 0x1,
        IL91874_RESOLUTION,             0,  4, width_msb, width_lsb, height_msb, height_lsb,
        IL91874_PDRF,                   0,  1, 0x00
    };
    // clang-format on
    qp_comms_bulk_command_sequence(device, il91874_init_sequence, sizeof(il91874_init_sequence));
    driver->base.rotation = rotation;

    if (driver->has_ram) {
        sram_init(device);
    }

    // clear gets the buffers correctly set to 0/1
    return driver->base.driver_vtable->clear(driver);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable

const struct eink_panel_dc_reset_painter_driver_vtable_t il91874_driver_vtable = {
    .base =
        {
            .init            = qp_il91874_init,
            .power           = qp_eink_panel_power,
            .clear           = qp_eink_panel_clear,
            .flush           = qp_eink_panel_flush,
            .pixdata         = qp_eink_panel_pixdata,
            .viewport        = qp_eink_panel_viewport,
            .palette_convert = qp_eink_panel_palette_convert,
            .append_pixels   = qp_eink_panel_append_pixels,
        },
    .swap_window_coords = false,
    .opcodes =
        {
            .display_on      = IL91874_POWER_ON,
            .display_off     = IL91874_POWER_OFF,
            .send_black_data = IL91874_SEND_BLACK,
            .send_red_data   = IL91874_SEND_RED,
            .refresh         = IL91874_DISPLAY_REFRESH,
        }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI

#ifdef QUANTUM_PAINTER_IL91874_SPI_ENABLE

// Factory functions for creating a handle to the IL91874 device
painter_device_t _qp_il91874_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, bool has_3color) {
    for (uint32_t i = 0; i < IL91874_NUM_DEVICES; ++i) {
        eink_panel_dc_reset_painter_device_t *driver = &il91874_drivers[i];
        if (!driver->base.driver_vtable) {
            driver->base.driver_vtable = (const struct painter_driver_vtable_t *)&il91874_driver_vtable;
            driver->base.comms_vtable  = (const struct painter_comms_vtable_t *)&spi_comms_with_dc_single_byte_vtable;
            /*
             * FIXME: May need an adjustment as each bit is really stored in 2 bits for 3-color displays
             *
             * However, this is fine (for now?) as the bits_per_pixel is only used on the underlying surface drivers
             * to memeset the buffer to 0, where 1 bit on each surface is still correct
             */
            driver->base.native_bits_per_pixel = 1;
            driver->base.panel_width           = panel_width;
            driver->base.panel_height          = panel_height;
            driver->base.rotation              = QP_ROTATION_0;
            driver->base.offset_x              = 0;
            driver->base.offset_y              = 0;

            driver->has_3color  = has_3color;
            driver->has_ram     = false;

            driver->timeout   = 3 * 60 * 1000; // 3 minutes as suggested by Adafruit
            driver->can_flush = true;

            driver->invert_black = false;
            driver->invert_red   = false;

            driver->black_surface = NULL;
            driver->red_surface   = NULL;

            driver->ram_opcodes.write_status = SRAM_23K640_WRITE_STATUS;
            driver->ram_opcodes.read_status  = SRAM_23K640_READ_STATUS;
            driver->ram_opcodes.write_data   = SRAM_23K640_WRITE;
            driver->ram_opcodes.read_data    = SRAM_23K640_READ;
            driver->ram_opcodes.sequential   = SRAM_23K640_SEQUENTIAL_MODE;
            driver->ram_chip_select_pin      = NO_PIN;

            // SPI and other pin configuration
            driver->base.comms_config                              = &driver->spi_dc_reset_config;
            driver->spi_dc_reset_config.spi_config.chip_select_pin = chip_select_pin;
            driver->spi_dc_reset_config.spi_config.divisor         = spi_divisor;
            driver->spi_dc_reset_config.spi_config.lsb_first       = false;
            driver->spi_dc_reset_config.spi_config.mode            = spi_mode;
            driver->spi_dc_reset_config.dc_pin                     = dc_pin;
            driver->spi_dc_reset_config.reset_pin                  = reset_pin;

            return (painter_device_t)driver;
        }
    }
    return NULL;
}
// No built-in RAM
painter_device_t qp_il91874_no_ram_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, bool has_3color, void *ptr) {
    painter_device_t device = _qp_il91874_make_spi_device(panel_width, panel_height, chip_select_pin, dc_pin, reset_pin, spi_divisor, spi_mode, has_3color);
    if (device) {
        eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;
        driver->black_surface = qp_make_mono1bpp_surface(panel_width, panel_height, ptr);
        driver->red_surface   = qp_make_mono1bpp_surface(panel_width, panel_height, ptr + EINK_BW_BYTES_REQD(panel_width, panel_height));
    }
    return device;
}

// Built-in RAM
painter_device_t qp_il91874_with_ram_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, bool has_3color, pin_t ram_chip_select_pin) {
    painter_device_t device = _qp_il91874_make_spi_device(panel_width, panel_height, chip_select_pin, dc_pin, reset_pin, spi_divisor, spi_mode, has_3color);
    if (device) {
        eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;
        driver->black_surface       = qp_make_0bpp_surface(panel_width, panel_height, NULL);
        driver->red_surface         = driver->black_surface;
        driver->ram_chip_select_pin = ram_chip_select_pin;
        driver->has_ram             = true;
    }
    return device;
}
#endif // QUANTUM_PAINTER_IL91874_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
