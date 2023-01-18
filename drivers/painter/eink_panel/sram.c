// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_eink_panel.h"
#include "qp_internal.h"
#include "qp_surface.h"
#include "spi_master.h"

/*
 * TODO: List
 *
 * Make sure dirty flags are always true
 * Implement data write/read functions
 */


void sram_init(painter_device_t device) {
    struct qp_comms_spi_config_t *comms_config = (struct qp_comms_spi_config_t *)driver->base.comms_config;

    // Start SPI
    spi_init();
    if (!spi_start(driver->ram_chip_select_pin, comms_config->lsb_first, comms_config->mode, comms_config->divisor)) {
        qp_dprintf("sram_init: error, couldnt start SPI\n");
        return false;
    }

    // Set up CS as output
    setPinOutput(driver->ram_chip_select_pin);

    // Init sequence
    writePinLow(driver->ram_chip_select_pin);
    for (int i=0; i<3; ++i) {
        spi_write(0xFF);
    }

    // End SPI
    writePinHigh(driver->ram_chip_select_pin);
    spi_stop();
}


bool sram_flush(painter_device_t device) {
}

inline uint16_t get_pixel(painter_device_t device) {
    struct surface_painter_device_t *surface  = (struct surface_painter_device_t *)driver->black_surface;
    return surface->pixdata_x + surface->pixdata_y * surface->base.panel_width;
}

inline void sram_send_info_to_address(uint16_t address, uint8_t *data, uint32_t length) {
    write_address(address);
    spi_transmit(data, index);
}

bool sram_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct painter_driver_t *                    black  = (struct painter_driver_t *)driver->black_surface;
    struct painter_driver_t *                    red    = (struct painter_driver_t *)driver->red_surface;

    // offset between black and red data
    uint32_t red_offset = EINK_BW_BYTES_REQD(driver->base.panel_width, driver->base.panel_height);

    // mini-buffers to avoid sending data in 1-byte transactions
    uint8_t black_data[32];
    uint8_t red_data[32];

    // both displays get drawn at the same position, getting the address on one of them is enough
    uint16_t pixel      = get_pixel(black);
    uint16_t byte       = pixel / 8;
    uint16_t start_byte = start_byte;

    // if we start in the middle of a byte, read its content to only modify the bits we want
    if ((pixel % 8) != 0) {
        black_data[0] = read_address(address);
        if (driver->has_3color) {
            red_data[0] = read_address(address + red_offset)
        }
    }

    for (uint32_t i = 0; i < native_pixel_count; ++i) {
        // add pixel info to array(s)
        bool black_bit = (pixels[i] >> 0) & 1;
        if (black_bit) {
            black_data[byte - start_byte] |=  (1 << (pixel % 8));
        } else {
            black_data[byte - start_byte] &= ~(1 << (pixel % 8));
        }
        if (driver->has_3color) {
            bool red_bit = (pixels[i] >> 1) & 1;
            if (red_bit) {
                red_data[byte - start_byte] |=  (1 << (pixel % 8));
            } else {
                red_data[byte - start_byte] &= ~(1 << (pixel % 8));
            }
        }

        // update position with dummy value, used to compute address
        black->driver_vtable->pixdata(driver->black_surface, (const void *)0, 1);
        pixel = get_pixel(black);

        // check if current pixel not on same nor next byte as previous one and there's info to send
        // code assumes always-increasing pixel position
        if ((pixel / 8  >  byte + 1) && (pixel - start_byte*8  !=  0)) {
            sram_send_info_to_address(address, black_data, n_bytes);

            if (driver->has_3color) {
                sram_send_info_to_address(address + red_offset, red_data, n_bytes);
            }

            // reset counters
            pixel      = get_pixel(black);
            byte       = pixel / 8;
            start_byte = byte;
        }
        byte  = pixel / 8;

        // check if we filled the buffers
        if ((byte - start_byte) == ARRAY_SIZE(black_data)) {
            sram_send_info_to_address(address, black_data, n_bytes);

            if (driver->has_3color) {
                sram_send_info_to_address(address + red_offset, red_data, n_bytes);
            }

            // reset counters
            pixel      = get_pixel(black);
            byte       = pixel / 8;
            start_byte = byte;
        }
    }

    return true;
}
