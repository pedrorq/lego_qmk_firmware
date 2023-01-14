// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_internal.h"
#include "qp_comms.h"
#include "qp_ssd1680.h"
#include "qp_ssd1680_opcodes.h"
#include "qp_eink_panel.h"
#include "qp_surface.h"

#ifdef QUANTUM_PAINTER_SSD1680_SPI_ENABLE
#    include <qp_comms_spi.h>
#endif // QUANTUM_PAINTER_SSD1680_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common

// Driver storage
eink_panel_dc_reset_painter_device_t ssd1680_drivers[SSD1680_NUM_DEVICES] = {0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization

bool qp_ssd1680_init(painter_device_t device, painter_rotation_t rotation) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    uint8_t width_lsb = (driver->base.panel_width-1) & 0xFF;
    uint8_t width_msb = ((driver->base.panel_width-1) >> 8) & 0xFF;
    // it makes a weird division by 8
    // uint8_t height    = (driver->base.panel_height/8 - 1) & 0xFF;

    /*
     * Values that change based on the variant (BW/3C, partial/full, MCU/builtin RAM)
     * - 2 color code based of: https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_213_BN.cpp
     * - 3 color code based of: https://github.com/adafruit/Adafruit_CircuitPython_SSD1680/blob/main/adafruit_ssd1680.py
     */
    bool is_bw = (driver->red_surface == NULL);
    uint8_t update_mode = is_bw ? 0xF8 : 0xF4;

    // clang-format off
    const uint8_t ssd1680_init_sequence[] = {
        // Command,                 Delay, N, Data[N]
        SSD1680_SOFT_RESET,           120, 0,
        SSD1680_RESOLUTION,             0, 3, width_lsb, width_msb, 0x00,
        SSD1680_DATA_ENTRY_MODE,        0, 1, 0x03,
        SSD1680_BORDER_CONTROL,         0, 1, 0x05,
        SSD1680_DISPLAY_UPDATE_CONTROL, 0, 2, 0x00, 0x80,
        SSD1680_TEMP_SENSOR,            0, 1, 0x80,
        SSD1680_UPDATE_MODE,            0, 1, update_mode,
        // SSD1680_VCOM_VOLTAGE,           0, 1, 0x36,
        // SSD1680_GATE_VOLTAGE,           0, 1, 0x17,
        // SSD1680_SOURCE_VOLTAGE,         0, 3, 0x41, 0x00, 0x32,
        // SSD1680_RAM_X_SIZE,             0, 2, 0x00, height,
        // SSD1680_RAM_X_COUNTER,          0, 1, 0x01,
        // SSD1680_RAM_Y_SIZE,             0, 4, width_lsb, width_msb, 0x00, 0x00,
        // SSD1680_RAM_Y_COUNTER,          0, 2, width_lsb, width_msb,
    };
    // clang-format on
    qp_comms_bulk_command_sequence(device, ssd1680_init_sequence, sizeof(ssd1680_init_sequence));
    driver->base.rotation = rotation;

    qp_init(driver->black_surface, rotation);

    if (driver->red_surface != NULL) {
        qp_init(driver->red_surface, rotation);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable

const struct eink_panel_dc_reset_painter_driver_vtable_t ssd1680_driver_vtable = {
    .base =
        {
            .init            = qp_ssd1680_init,
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
            .display_on  = SSD1680_NOP, // Couldnt find a turn-on command
            .display_off = SSD1680_NOP, // There is a cmd to go into sleep mode, but requires HW reset in order to wake up
            .send_black_data = SSD1680_SEND_BLACK,
            .send_red_data = SSD1680_SEND_RED,
            .refresh = SSD1680_DISPLAY_REFRESH,
        }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI

#ifdef QUANTUM_PAINTER_SSD1680_SPI_ENABLE

// Factory function for creating a handle to the SSD1680 device
painter_device_t qp_ssd1680_bw_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, void *ptr) {
    for (uint32_t i = 0; i < SSD1680_NUM_DEVICES; ++i) {
        eink_panel_dc_reset_painter_device_t *driver = &ssd1680_drivers[i];
        if (!driver->base.driver_vtable) {
            driver->base.driver_vtable         = (const struct painter_driver_vtable_t *)&ssd1680_driver_vtable;
            driver->base.comms_vtable          = (const struct painter_comms_vtable_t *)&spi_comms_with_dc_single_byte_vtable;
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

            driver->black_surface = qp_make_mono1bpp_surface(panel_width, panel_height, ptr);
            driver->red_surface   = NULL;

            driver->timeout   = 2 * 60 * 1000; // 2 minutes as seen on WeAct
            driver->can_flush = true;

            driver->invert_black = true;
            driver->invert_red   = false;

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

painter_device_t qp_ssd1680_3c_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, void *ptr) {
    painter_device_t device = qp_ssd1680_bw_make_spi_device(panel_width, panel_height, chip_select_pin, dc_pin, reset_pin, spi_divisor, spi_mode, ptr);
    if (device) {
        eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;
        driver->red_surface = qp_make_mono1bpp_surface(panel_width, panel_height, ptr+EINK_BW_BYTES_REQD(panel_width, panel_height));
    }
    return device;
}
#endif // QUANTUM_PAINTER_SSD1680_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
