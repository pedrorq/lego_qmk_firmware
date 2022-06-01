/* Copyright 2021-2022 alin m elena <alinm.elena@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define PRODUCT_ID 0x0001
#define DEVICE_VER 0x6071

#define MATRIX_ROW_PINS \
    { B7, B6, B5, B4, B3, A15, A10, A8 }
#define MATRIX_COL_PINS \
    { B10, B1, B0, A3, A2, A1, A0, C15, C14, C13, B9, B8, B12, B13, B14, B15 }

/* This Shift Register expands available hardware output lines to control additional peripherals */
/* It uses four lines from the MCU to provide 16 output lines */
/* Shift Register Clock configuration (MCU to ShiftRegister.RCLK) */
//#define SR_EXP_RCLK_PIN          B14
#define SR_EXP_RCLK_PIN          A6
/* Shift Register Output Enable configuration (MCU to ShiftRegister.OE_N) */
//#define SR_EXP_OE_PIN            B15
#define SR_EXP_OE_PIN            A7
/* SERCOM port to use for Shift Register SPI */
/* DATAOUT and SCLK must be configured to use hardware pins of this port */
#define SPI_SERCOM               SERCOM2
/* Shift Register SPI Data Out configuration (MCU.SERCOMx.PAD[0] to ShiftRegister.SER) */
//#define SPI_DATAOUT_PIN          A12
#define SPI_DATAOUT_PIN          A4
#define SPI_DATAOUT_MUX          2
/* Shift Register SPI Serial Clock configuration (MCU.SERCOMx.PAD[1] to ShiftRegister.SRCLK) */
//#define SPI_SCLK_PIN             A13
#define SPI_SCLK_PIN             A5
#define SPI_SCLK_MUX             2


#define UNUSED_PINS

/* COL2ROW, ROW2COL*/
#define DIODE_DIRECTION COL2ROW

#define MATRIX_IO_DELAY 5


