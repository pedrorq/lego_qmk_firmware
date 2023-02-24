// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "config_common.h"

#if !defined(INIT_EE_HANDS_LEFT) && !defined(INIT_EE_HANDS_RIGHT)
#    error "You need to configure EE_HANDS"
#endif

#define MATRIX_ROWS 10
#define MATRIX_COLS 8

// =======
// Debugging
// #define CUSTOM_SPI_DEBUG
// #define DEBUG_MATRIX_SCAN_RATE
#define INIT_DELAY 3000
#define QUANTUM_PAINTER_DEBUG
// #define TOUCH_SCREEN_DEBUG
// this is slow as f
// #define SIPO_PINS_DEBUG

// =======
// Power indicator
#define POWER_LED_PIN GP25

// =======
// SPI config
#define SPI_COUNT 2

#define SCREENS_SPI_DRIVER SPID1
#define SCREENS_SCK_PIN GP10
#define SCREENS_MOSI_PIN GP11
#define SCREENS_MISO_PIN GP12
#define SCREENS_SPI_MODE 0
#define SCREENS_SPI_DIV 0

#define REGISTERS_SPI_DRIVER SPID0
#define REGISTERS_SCK_PIN GP2
#define REGISTERS_MOSI_PIN GP3
#define REGISTERS_MISO_PIN GP4
#define PISO_CS_PIN GP1
#define SIPO_CS_PIN GP13
#define REGISTERS_SPI_MODE 0
#define REGISTERS_SPI_DIV  0
#define N_SIPO_PINS 8
#define DUMMY_PIN GP14

// =======
// Displays
#define IL91874_ROTATION 0
#define ILI9163_ROTATION 0
#define ILI9341_ROTATION 2

// =======
// RGB
#define RGB_DI_PIN GP0
#define RGB_DISABLE_WHEN_USB_SUSPENDED
#define RGB_MATRIX_LED_COUNT 16
#define WS2812_PIO_USE_PIO1

// =======
// Split
#define EE_HANDS
#define USE_SERIAL
#define SERIAL_USART_TX_PIN GP15
