// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_internal.h"
#include "qp_comms.h"
#include "qp_il91874.h"
#include "qp_il91874_opcodes.h"
#include "qp_eink_panel.h"
#include "qp_surface.h"
#include "qp_surface_internal.h"
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
    eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;

    uint16_t width    = driver->base.panel_width;
    uint16_t height   = driver->base.panel_height;

    uint8_t width_lsb  = width & 0xFF;
    uint8_t width_msb  = (width >> 8) & 0xFF;
    uint8_t height_lsb = height & 0xFF;
    uint8_t height_msb = (height >> 8) & 0xFF;

    // panel config for no mirroring, 3color, 320x300
    uint8_t config = 0x2F;

    // this bit controls whether display is B/W or B/W/R
    // we defaulted to 0 (BWR), set it high if BW
    if (!driver->has_3color)
        config |= (1 << 4);

    // clang-format off
    const uint8_t il91874_init_sequence[] = {
        // Command,                 Delay,  N, Data[N]
        IL91874_POWER_ON,             120,  0,
        IL91874_PANEL_SETTING,          0,  1, config,
        IL91874_PLL,                    0,  1, 0x3A,
        IL91874_POWER_SETTING,          0,  5, 0x03, 0x00, 0x2B, 0x2B, 0x09,
        IL91874_BOOSTER_SOFT_START,     0,  3, 0x07, 0x07, 0x17,
        IL91874_MISTERY_COMMAND,        0,  2, 0x60, 0xA5,
        IL91874_MISTERY_COMMAND,        0,  2, 0x89, 0xA5,
        IL91874_MISTERY_COMMAND,        0,  2, 0x90, 0x00,
        IL91874_MISTERY_COMMAND,        0,  2, 0x93, 0xA2,
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

    // clear gets the buffers correctly set to 0/1
    return driver->base.driver_vtable->clear(device);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Patched functions
bool qp_il91874_no_sram_flush(painter_device_t device) {
    eink_panel_dc_reset_painter_device_t *       driver  = (eink_panel_dc_reset_painter_device_t *)device;
    eink_panel_dc_reset_painter_driver_vtable_t *vtable  = (eink_panel_dc_reset_painter_driver_vtable_t *)driver->base.driver_vtable;
    surface_painter_device_t *                   black   = (surface_painter_device_t *)driver->black_surface;
    uint32_t                                     n_bytes = EINK_BW_BYTES_REQD(driver->base.panel_width, driver->base.panel_height);

    // 3 color is handled by default code
    if (driver->has_3color) {
        qp_dprintf("qp_il91874_no_sram_flush: BWR mode, fallback to qp_eink_panel_flush\n");
        return qp_eink_panel_flush(device);
    }

    // else, we need to send black data with the red_data opcode
    if (!black->dirty.is_dirty) {
        qp_dprintf("qp_il91874_no_sram_flush: done (no changes to be sent)\n");
        return true;
    }

    if (!driver->can_flush) {
        qp_dprintf("qp_il91874_no_sram_flush: fail (can_flush == false)\n");
        return false;
    }

    qp_comms_command(device, vtable->opcodes.send_red_data);
    qp_comms_send(device, black->buffer, n_bytes);

    qp_comms_command(device, vtable->opcodes.refresh);

    qp_eink_update_can_flush(device);

    return true;
}

bool qp_il91874_with_sram_init(painter_device_t device, painter_rotation_t rotation) {
    bool ret = qp_il91874_init(device, rotation);
    sram_init(device);
    return ret;
}

bool qp_il91874_with_sram_clear(painter_device_t device) {
    bool ret = qp_eink_panel_clear(device);
    sram_init(device);
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable

const eink_panel_dc_reset_painter_driver_vtable_t il91874_no_sram_driver_vtable = {
    .base =
        {
            .init            = qp_il91874_init,
            .power           = qp_eink_panel_power,
            .clear           = qp_eink_panel_clear,
            .flush           = qp_il91874_no_sram_flush,
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

const eink_panel_dc_reset_painter_driver_vtable_t il91874_with_sram_driver_vtable = {
    .base =
        {
            .init            = qp_il91874_with_sram_init,
            .power           = qp_eink_panel_power,
            .clear           = qp_il91874_with_sram_clear,
            .flush           = sram_flush,
            .pixdata         = sram_pixdata,
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

// No built-in RAM
painter_device_t qp_il91874_no_sram_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, bool has_3color, void *ptr) {
    for (uint32_t i = 0; i < IL91874_NUM_DEVICES; ++i) {
        eink_panel_dc_reset_painter_device_t *driver = &il91874_drivers[i];
        if (!driver->base.driver_vtable) {
            driver->base.driver_vtable = (const painter_driver_vtable_t *)&il91874_no_sram_driver_vtable;
            driver->base.comms_vtable  = (const painter_comms_vtable_t *)&spi_comms_with_dc_single_byte_vtable;

            driver->base.native_bits_per_pixel = 1;
            driver->base.panel_width           = panel_width;
            driver->base.panel_height          = panel_height;
            driver->base.rotation              = QP_ROTATION_0;
            driver->base.offset_x              = 0;
            driver->base.offset_y              = 0;

            driver->timeout   = 3 * 60 * 1000; // 3 minutes as suggested by Adafruit
            driver->can_flush = true;

            driver->has_3color   = has_3color;
            driver->invert_black = false;
            driver->invert_red   = false;

            driver->black_surface   = qp_make_mono1bpp_surface(panel_width, panel_height, ptr);
            if (driver->has_3color)
                driver->red_surface = qp_make_mono1bpp_surface(panel_width, panel_height, ptr + EINK_BW_BYTES_REQD(panel_width, panel_height));
            else
                driver->red_surface = qp_make_empty0bpp_surface(panel_width, panel_height);

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

// Built-in RAM
painter_device_t qp_il91874_with_sram_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, bool has_3color, pin_t sram_chip_select_pin) {
    for (uint32_t i = 0; i < IL91874_NUM_DEVICES; ++i) {
        eink_panel_dc_reset_painter_device_t *          driver = &il91874_drivers[i];
        eink_panel_dc_reset_with_sram_painter_device_t *drv    = (eink_panel_dc_reset_with_sram_painter_device_t *)driver;
        if (!driver->base.driver_vtable) {
            driver->base.driver_vtable = (const painter_driver_vtable_t *)&il91874_with_sram_driver_vtable;
            driver->base.comms_vtable  = (const painter_comms_vtable_t *)&spi_comms_with_dc_single_byte_vtable;

            driver->base.native_bits_per_pixel = 1;
            driver->base.panel_width           = panel_width;
            driver->base.panel_height          = panel_height;
            driver->base.rotation              = QP_ROTATION_0;
            driver->base.offset_x              = 0;
            driver->base.offset_y              = 0;

            driver->timeout   = 3 * 60 * 1000; // 3 minutes as suggested by Adafruit
            driver->can_flush = true;

            driver->has_3color   = has_3color;
            driver->invert_black = false;
            driver->invert_red   = false;

            // We dont store pixels on the MCU's RAM, we simply want to keep track of the position in which to draw
            // Since each pixel will be drawn on the same position for both color channels, one surface is enough
            driver->black_surface = qp_make_empty0bpp_surface(panel_width, panel_height);
            driver->red_surface   = driver->black_surface;

            // SPI and other pin configuration
            driver->base.comms_config                              = &driver->spi_dc_reset_config;
            driver->spi_dc_reset_config.spi_config.chip_select_pin = chip_select_pin;
            driver->spi_dc_reset_config.spi_config.divisor         = spi_divisor;
            driver->spi_dc_reset_config.spi_config.lsb_first       = false;
            driver->spi_dc_reset_config.spi_config.mode            = spi_mode;
            driver->spi_dc_reset_config.dc_pin                     = dc_pin;
            driver->spi_dc_reset_config.reset_pin                  = reset_pin;
            drv->sram_chip_select_pin                              = sram_chip_select_pin;

            return (painter_device_t)driver;
        }
    }
    return NULL;
}
#endif // QUANTUM_PAINTER_IL91874_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
