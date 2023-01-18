// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qp_comms_spi.h"
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
bool sram_spi_start(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *driver       = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct qp_comms_spi_config_t *               comms_config = (struct qp_comms_spi_config_t *)driver->base.comms_config;

    spi_init();

    // close SPI, in case it was init'ed before
    spi_stop();

    if (!spi_start(driver->ram_chip_select_pin, comms_config->lsb_first, comms_config->mode, comms_config->divisor)) {
        qp_dprintf("sram_spi_start: error, couldnt start SPI\n");
        return false;
    }

    // Set up CS as output high
    setPinOutput(driver->ram_chip_select_pin);
    writePinHigh(driver->ram_chip_select_pin);

    return true;
}

// write/read functions assume SPI is initialized & started already
uint8_t sram_read_byte(painter_device_t device, uint16_t address) {
    struct eink_panel_dc_reset_painter_device_t *driver  = (struct eink_panel_dc_reset_painter_device_t *)device;
    uint8_t                                      array[] = {driver->ram_opcodes.read_data, address >> 8, address & 0xFF};

    // set up read mode and address
    writePinLow(driver->ram_chip_select_pin);
    spi_transmit(array, ARRAY_SIZE(array));

    // read a byte
    uint8_t ret = spi_write(0x00);

    // reset CS
    writePinHigh(driver->ram_chip_select_pin);

    return ret;
}

void sram_write(painter_device_t device, uint16_t address, uint8_t *data, uint16_t length) {
    struct eink_panel_dc_reset_painter_device_t *driver  = (struct eink_panel_dc_reset_painter_device_t *)device;
    uint8_t                                      array[] = {driver->ram_opcodes.write_data, address >> 8, address & 0xFF};

    // set up write mode and address
    writePinLow(driver->ram_chip_select_pin);
    spi_transmit(array, ARRAY_SIZE(array));

    // send data
    spi_transmit(data, length);

    // reset CS
    writePinHigh(driver->ram_chip_select_pin);
}

void sram_spi_stop(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;

    writePinHigh(driver->ram_chip_select_pin);
    spi_stop();
}

void sram_init(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *driver  = (struct eink_panel_dc_reset_painter_device_t *)device;
    uint8_t                                      array[] = {driver->ram_opcodes.write_status, driver->ram_opcodes.sequential >> 8, driver->ram_opcodes.sequential & 0xFF};

    // Start SPI
    if (!sram_spi_start(device)) {
        qp_dprintf("sram_init: fail\n");
        return;
    }

    // Init sequence
    writePinLow(driver->ram_chip_select_pin);
    for (int i=0; i<3; ++i) {
        spi_write(0xFF);
    }

    // Setup sequential mode
    spi_transmit(array, ARRAY_SIZE(array));

    sram_spi_stop(device);
}

void sram_to_display(painter_device_t device, bool black) {
    struct eink_panel_dc_reset_painter_device_t *       driver       = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable       = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->base.driver_vtable;
    struct qp_comms_spi_dc_reset_config_t *             comms_config = (struct qp_comms_spi_dc_reset_config_t *)driver->base.comms_config;
    uint32_t                                            n_bytes      = EINK_BW_BYTES_REQD(driver->base.panel_width, driver->base.panel_height);
    uint32_t                                            address      = black ? 0 : n_bytes;
    uint8_t                                             cmd          = black ? vtable->opcodes.send_black_data : vtable->opcodes.send_red_data;

    // Select SRAM and put it on read mode at data's address
    writePinLow(driver->ram_chip_select_pin);
    uint8_t array[] = {driver->ram_opcodes.read_data, (address >> 8) & 0xFF, address & 0xFF};
    spi_transmit(array, ARRAY_SIZE(array));

    // Sending write command to display will also trigger the SRAM to send the 1st byte on MISO
    writePinLow(comms_config->spi_config.chip_select_pin);
    writePinLow(comms_config->dc_pin);
    uint8_t byte = spi_write(cmd);
    writePinHigh(comms_config->spi_config.chip_select_pin);

    // Send data
    writePinHigh(comms_config->dc_pin);
    for (uint32_t i=0; i<n_bytes; ++i) {
        writePinLow(comms_config->spi_config.chip_select_pin);
        byte = spi_write(byte);
        writePinHigh(comms_config->spi_config.chip_select_pin);
    }
    writePinHigh(driver->ram_chip_select_pin);
}

void sram_flush(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *       driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct eink_panel_dc_reset_painter_driver_vtable_t *vtable = (struct eink_panel_dc_reset_painter_driver_vtable_t *)driver->base.driver_vtable;

    if (!sram_spi_start(device)) {
        qp_dprintf("sram_flush: fail\n");
        return;
    }

    // Stream black and red data
    sram_to_display(device, true);
    sram_to_display(device, false);

    writePinLow(driver->spi_dc_reset_config.spi_config.chip_select_pin);
    writePinLow(driver->spi_dc_reset_config.dc_pin);
    spi_write(vtable->opcodes.refresh);
    writePinHigh(driver->spi_dc_reset_config.spi_config.chip_select_pin);

    sram_spi_stop(device);
}

uint16_t get_pixel(painter_device_t device) {
    struct eink_panel_dc_reset_painter_device_t *driver  = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct surface_painter_device_t *            surface = (struct surface_painter_device_t *)driver->black_surface;

    return surface->pixdata_x + surface->pixdata_y * surface->base.panel_width;
}

bool sram_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count) {
    struct eink_panel_dc_reset_painter_device_t *driver = (struct eink_panel_dc_reset_painter_device_t *)device;
    struct painter_driver_t *                    black  = (struct painter_driver_t *)driver->black_surface;
    uint8_t *                                    pixels = (uint8_t *)pixel_data;

    // offset between black and red data
    uint32_t red_offset = EINK_BW_BYTES_REQD(driver->base.panel_width, driver->base.panel_height);

    // mini-buffers to avoid sending data in 1-byte transactions
    uint8_t black_data[32];
    uint8_t red_data[32];

    // both displays get drawn at the same position, getting the address on one of them is enough
    uint16_t pixel   = get_pixel(black);
    uint16_t byte    = pixel / 8;
    uint8_t  bit     = pixel % 8;
    uint16_t address = byte;

    if (!sram_spi_start(device)) {
        qp_dprintf("sram_pixdata: fail\n");
        return false;
    }

    // if we start in the middle of a byte, read its content to only modify the bits we want
    if ((pixel % 8) != 0) {
        black_data[0] = sram_read_byte(device, address);
        if (driver->has_3color) {
            red_data[0] = sram_read_byte(device, address + red_offset);
        }
    }

    for (uint32_t i = 0; i < native_pixel_count; ++i) {
        // add pixel info to buffer(s)
        bool black_bit = (pixels[i] >> 0) & 1;
        if (black_bit) {
            black_data[byte - address] |=  (1 << bit);
        } else {
            black_data[byte - address] &= ~(1 << bit);
        }
        if (driver->has_3color) {
            bool red_bit = (pixels[i] >> 1) & 1;
            if (red_bit) {
                red_data[byte - address] |=  (1 << (pixel % 8));
            } else {
                red_data[byte - address] &= ~(1 << (pixel % 8));
            }
        }

        // update position with dummy value, used to compute address
        black->driver_vtable->pixdata(driver->black_surface, (const void *)0, 1);
        pixel = get_pixel(black);

        // check if current pixel not on same nor next byte as previous one and there's info to send
        // code assumes always-increasing pixel position
        if ((pixel / 8  >  byte + 1) && (pixel - address*8  !=  0)) {
            sram_write(device, address, black_data, byte - address);

            if (driver->has_3color) {
                sram_write(device, address + red_offset, red_data, byte - address);
            }

            // reset counters
            pixel   = get_pixel(black);
            byte    = pixel / 8;
            address = byte;
        }
        // updated after pixel so we can check byte continuity
        byte = pixel / 8;

        // check if we filled the buffers
        if ((byte - address) == ARRAY_SIZE(black_data)) {
            sram_write(device, address, black_data, byte - address);

            if (driver->has_3color) {
                sram_write(device, address + red_offset, red_data, byte - address);
            }

            // reset counters
            pixel   = get_pixel(black);
            byte    = pixel / 8;
            bit     = pixel % 8;
            address = byte;
        }
    }

    sram_spi_stop(device);

    return true;
}
