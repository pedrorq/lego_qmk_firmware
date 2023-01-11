/* Copyright 2020 Nick Brassel (tzarc)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <ch.h>
#include <hal.h>
#include <stdbool.h>

#include "gpio.h"
#include "chibios_config.h"

#include "cpp_map.h"

#ifdef CUSTOM_SPI_DEBUG
#    include <debug.h>
#    include <print.h>
#    include <wait.h>
#    define spi_dprintf(...) dprintf(__VA_ARGS__); wait_ms(30)
#else
#    define spi_dprintf(...) do { } while (0)
#endif

#if !defined(SPI_DRIVERS)
#    pragma message "Select drivers for SPI"
#endif

#define TO_PTR(x) (&(x)),
#define _SPI_DRIVERS MAP(TO_PTR, SPI_DRIVERS)

#if !defined(SPI_SCK_PINS)
#    pragma message "Select pins for SCK"
#endif

#ifndef SPI_SCK_PAL_MODE
#    if defined(USE_GPIOV1)
#        define SPI_SCK_PAL_MODE PAL_MODE_ALTERNATE_PUSHPULL
#    else
#        define SPI_SCK_PAL_MODE 5
#    endif
#endif

#if !defined(SPI_MOSI_PINS)
#    pragma message "Select pins for MOSI"
#endif

#ifndef SPI_MOSI_PAL_MODE
#    if defined(USE_GPIOV1)
#        define SPI_MOSI_PAL_MODE PAL_MODE_ALTERNATE_PUSHPULL
#    else
#        define SPI_MOSI_PAL_MODE 5
#    endif
#endif

#ifndef SPI_MISO_PINS
#    pragma message "Select pins for MISO"
#endif

#ifndef SPI_MISO_PAL_MODE
#    if defined(USE_GPIOV1)
#        define SPI_MISO_PAL_MODE PAL_MODE_ALTERNATE_PUSHPULL
#    else
#        define SPI_MISO_PAL_MODE 5
#    endif
#endif

typedef int16_t spi_status_t;

#define SPI_STATUS_SUCCESS (0)
#define SPI_STATUS_ERROR (-1)
#define SPI_STATUS_TIMEOUT (-2)

#define SPI_TIMEOUT_IMMEDIATE (0)
#define SPI_TIMEOUT_INFINITE (0xFFFF)

#ifdef __cplusplus
extern "C" {
#endif
void custom_spi_init(uint8_t index);

bool custom_spi_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor, uint8_t index);

spi_status_t custom_spi_write(uint8_t data, uint8_t index);

spi_status_t custom_spi_read(uint8_t index);

spi_status_t custom_spi_transmit(const uint8_t *data, uint16_t length, uint8_t index);

spi_status_t custom_spi_receive(uint8_t *data, uint16_t length, uint8_t index);

void custom_spi_stop(uint8_t index);
#ifdef __cplusplus
}
#endif
