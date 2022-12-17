// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_internal.h"
#include "qp_comms.h"
#include "qp_il91874.h"
#include "qp_il91874_opcodes.h"
#include "qp_eink_panel.h"

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
    struct painter_driver_t *driver = (struct painter_driver_t *)device;

    // clang-format off
    const uint8_t il91874_init_sequence[] = {
        // Command,                 Delay,  N, Data[N]
        IL91874_POWER_ON,             255,  0,
        IL91874_PANEL_SETTING,          0,  1, 0xAF,
        IL91874_PLL,                    0,  1, 0x3A,
        IL91874_POWER_SETTING,          0,  5, 0x03, 0x00, 0x2B, 0x2B, 0x09,
        IL91874_BOOSTER_SOFT_START,     0,  3, 0x07, 0x07, 0x17,
        IL91874_VCM_DC_SETTING,         0,  1, 0x12,
        IL91874_CDI,                    0,  1, 0x87,
        IL91874_RESOLUTION,             0,  4, (driver->panel_width >> 8) & 0xFF, (driver->panel_width) & 0xFF, (driver->panel_height >> 8) & 0xFF, (driver->panel_height) & 0xFF,
        IL91874_PDRF,                   0,  1, 0x00
    };
    // clang-format on
    qp_comms_bulk_command_sequence(device, il91874_init_sequence, sizeof(il91874_init_sequence));

    return true;
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
            .palette_convert = qp_eink_panel_palette_convert_eink3,
            .append_pixels   = qp_eink_panel_append_pixels_eink3,
        },
    .num_window_bytes   = 2,
    .swap_window_coords = false,
    .opcodes =
        {
            .display_on  = IL91874_POWER_ON,
            .display_off = IL91874_POWER_OFF,
            .send_black_data = IL91874_SEND_BLACK,
            .send_red_data = IL91874_SEND_RED,
            .refresh     = IL91874_DISPLAY_REFRESH,
        },
    .has_3_colors = true,
    .has_partial_refresh = false,
    .has_ram = false
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI

#ifdef QUANTUM_PAINTER_IL91874_SPI_ENABLE

// Factory function for creating a handle to the IL91874 device
painter_device_t qp_il91874_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode) {
    for (uint32_t i = 0; i < IL91874_NUM_DEVICES; ++i) {
        eink_panel_dc_reset_painter_device_t *driver = &il91874_drivers[i];
        if (!driver->base.driver_vtable) {
            driver->base.driver_vtable         = (const struct painter_driver_vtable_t *)&il91874_driver_vtable;
            driver->base.comms_vtable          = (const struct painter_comms_vtable_t *)&spi_comms_with_dc_vtable;
            driver->base.native_bits_per_pixel = 1; // Each pixel by 2 bits (3 color), but code is prepared for that, so this value is correct
            driver->base.panel_width  = panel_width;
            driver->base.panel_height = panel_height;
            driver->base.rotation     = QP_ROTATION_0;
            driver->base.offset_x     = 0;
            driver->base.offset_y     = 0;
            uint8_t *ptr = (uint8_t *) malloc(panel_width * panel_height / 8 * 2);
            if (ptr == NULL) {
                qp_dprintf("Couldn't allocate memory for eink buffer\n");
            } else {
                driver->framebuffer = ptr;
            }

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

#endif // QUANTUM_PAINTER_IL91874_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
