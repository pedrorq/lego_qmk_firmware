// Copyright 2023 Pablo Martinez (@elpekenin)
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

#define LUT_SIZE  153

#define PART_UPDATE_LUT   0x0 , 0x40, 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x80, 0x80, 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x40, 0x40, 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x80, 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0A, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x2, 0x1, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0, 0x0, 0x0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common

// Driver storage
eink_panel_dc_reset_painter_device_t ssd1680_drivers[SSD1680_NUM_DEVICES] = {0};

static inline void wait_for_busy(pin_t pin){
  while(readPin(pin)) {};
}

static inline void hw_reset(pin_t pin) {
  writePinHigh(pin);
  wait_ms(200);
  writePinLow(pin);
  wait_ms(200);
  writePinHigh(pin);
  wait_ms(200);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization

bool qp_ssd1680_init(painter_device_t device, painter_rotation_t rotation) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
// longest directon
    uint8_t y_lsb = (driver->base.panel_height-1) &0xFF;
    uint8_t y_msb = ((driver->base.panel_height-1) >> 8) & 0xFF;
//shortest direction
    // it makes a weird division by 8
    uint8_t x    = ((driver->base.panel_width-1)>>3) & 0xFF ;

    /*
     * Values that change based on the variant (BW/3C, partial/full, MCU/builtin RAM)
     * - 2 color code based of: https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_213_BN.cpp
     * - 3 color code based of: https://github.com/adafruit/Adafruit_CircuitPython_SSD1680/blob/main/adafruit_ssd1680.py
     */

//if (driver->has_partial){
 // clang-format off
    // partial update
  /*
      const uint8_t ssd1680_init_sequence[] = {
        // Command,                       Delay, N, Data[0],Data[1],...,Data[N-1]
        SSD1680_SOFT_RESET                    , 250 , 0   , //0x12
        SSD1680_DRIVER_OUTPUT_CONTROL         , 250 , 3   , 0x27, 0x01, 0x00, //0x01
        SSD1680_DATA_ENTRY_MODE               , 0   , 1   , 0x03, //0x11
        SSD1680_RAM_X_SIZE                    , 0   , 2   , 0x00, x   , //0x44
        SSD1680_RAM_Y_SIZE                    , 0   , 4   , 0x00, 0x00, y_msb, y_lsb, //0x45
        SSD1680_RAM_X_COUNTER                 , 0   , 1   , 0x00, //0x4E
        SSD1680_RAM_Y_COUNTER                 , 0   , 2   , 0x00, 0x00, //0x4F
        SSD1680_BORDER_CONTROL                , 0   , 1   , 0x80, //0x3C
        SSD1680_TEMP_SENSOR                   , 0   , 1   , 0x80, //0x18
        SSD1680_WRITE_LUT_REGISTER            , 250 , LUT_SIZE , PART_UPDATE_LUT, //0x32
        SSD1680_END_OPTION,                        0,    1, 0x22, //0x3f
        SSD1680_GATE_DRIVING_VOLTAGE_CONTROL  , 0   , 1   , 0x17, //0x03
        SSD1680_SOURCE_DRIVING_VOLTAGE_CONTROL, 0   , 3   , 0x41, 0x0 , 0x32, //0x04
        SSD1680_WRITE_VCOM_REGISTER           , 0   , 1   , 0x32, //0x2C
        SSD1680_WRITE_REGISTER_DISPLAY_OPTION, 0 , 10, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x40 , 0x0, 0x0, 0x0, 0x0,  //0x37
        SSD1680_DISPLAY_UPDATE_CONTROL        , 250 , 1   , 0xC0, //0x22
        SSD1680_ACTIVATE_DISPLAY_UPDATE       , 250 , 0   , //0x20
    };
    */
    // clang-format on
//} else{
        // clang-format off
// no partial update
       const uint8_t ssd1680_init_sequence[] = {
//        Command,                       Delay, N, Data[0],Data[1],...,Data[N-1]
       SSD1680_SOFT_RESET                    , 250 , 0   , //0x12
       SSD1680_DRIVER_OUTPUT_CONTROL         , 250 , 3   , 0x27, 0x01, 0x00, //0x01
       SSD1680_DATA_ENTRY_MODE               , 0   , 1   , 0x03, //0x11
       SSD1680_RAM_X_SIZE                    , 0   , 2   , 0x00, x   , //0x44
       SSD1680_RAM_Y_SIZE                    , 0   , 4   , 0x00, 0x00, y_msb, y_lsb, //0x45
       SSD1680_RAM_X_COUNTER                 , 0   , 1   , 0x00, //0x4E
       SSD1680_RAM_Y_COUNTER                 , 0   , 2   , 0x00, 0x00, //0x4F
       SSD1680_BORDER_CONTROL                , 0   , 1   , 0x80, //0x3C
       SSD1680_TEMP_SENSOR                   , 0   , 1   , 0x80, //0x18
       SSD1680_DISPLAY_UPDATE_CONTROL_RAM,   0, 2, 0x00,0x80, //0x21
       SSD1680_DISPLAY_UPDATE_CONTROL,0,1,0xf8,              //0x22
       SSD1680_ACTIVATE_DISPLAY_UPDATE       , 250 , 0   , //0x20
   };
//};

    // this is hw reset according to the datasheet
    hw_reset(EINK_RST_PIN);
    qp_comms_bulk_command_sequence(device, ssd1680_init_sequence, sizeof(ssd1680_init_sequence));
    driver->base.rotation = rotation;

    // clear gets the buffers correctly set to 0/1
  bool ret  = driver->base.driver_vtable->clear(driver);
return ret;
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
            .palette_convert = qp_eink_panel_palette_convert,
            .append_pixels   = qp_eink_panel_append_pixels,
        },
    .opcodes =
        {
            .display_on  = SSD1680_NOP, // Couldnt find a turn-on command
            .display_off = SSD1680_NOP, // There is a cmd to go into sleep mode, but requires HW reset in order to wake up
            .send_black_data = SSD1680_SEND_BLACK,
            .send_red_data = SSD1680_SEND_RED,
            .refresh = SSD1680_ACTIVATE_DISPLAY_UPDATE,
        }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI

#ifdef QUANTUM_PAINTER_SSD1680_SPI_ENABLE

// Factory function for creating a handle to the SSD1680 device
painter_device_t qp_ssd1680_bw_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, bool has_color, bool has_partial, void *ptr) {
    for (uint32_t i = 0; i < SSD1680_NUM_DEVICES; ++i) {
        eink_panel_dc_reset_painter_device_t *driver = &ssd1680_drivers[i];
        if (!driver->base.driver_vtable) {
            driver->base.driver_vtable = (const struct painter_driver_vtable_t *)&ssd1680_driver_vtable;
            driver->base.comms_vtable  = (const struct painter_comms_vtable_t *)&spi_comms_with_dc_single_byte_vtable;

            driver->base.native_bits_per_pixel = 1;
            driver->base.panel_width           = panel_width;
            driver->base.panel_height          = panel_height;
            driver->base.rotation              = QP_ROTATION_0;
            driver->base.offset_x              = 0;
            driver->base.offset_y              = 0;

            driver->black_surface = qp_make_mono1bpp_surface(panel_width, panel_height, ptr);
            driver->red_surface = qp_make_mono1bpp_surface(panel_width, panel_height, ptr + EINK_BW_BYTES_REQD(panel_width, panel_height));
            // 0bpp needs changes to not ask for a pointer, so far we'll just set it to NULL

            driver->has_3color = has_color;
            driver->has_partial = has_partial;
            //driver->has_ram    = false;

            driver->timeout   = 6000; // 2 minutes as seen on WeAct
            driver->can_flush = true;

            driver->invert_black = true;
            driver->invert_red   = false;

            // SPI and other pin configuration
            driver->base.comms_config                              = &driver->spi_dc_reset_config;
            driver->spi_dc_reset_config.spi_config.chip_select_pin = chip_select_pin;
            driver->spi_dc_reset_config.spi_config.divisor         = spi_divisor;
            driver->spi_dc_reset_config.spi_config.lsb_first       = true;
            driver->spi_dc_reset_config.spi_config.mode            = spi_mode;
            driver->spi_dc_reset_config.dc_pin                     = dc_pin;
            driver->spi_dc_reset_config.reset_pin                  = reset_pin;

            return (painter_device_t)driver;
        }
    }
    return NULL;
}

painter_device_t qp_ssd1680_3c_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode, bool has_partial, void *ptr) {
    painter_device_t device = qp_ssd1680_bw_make_spi_device(panel_width, panel_height, chip_select_pin, dc_pin, reset_pin, spi_divisor, spi_mode, false, has_partial, ptr);
    if (device) {
        eink_panel_dc_reset_painter_device_t *driver = (eink_panel_dc_reset_painter_device_t *)device;
        driver->has_3color                           = true;

    }
    return device;
}
#endif // QUANTUM_PAINTER_SSD1680_SPI_ENABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
