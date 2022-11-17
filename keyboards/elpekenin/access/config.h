// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "config_common.h"

// =======
// Power indicator
#define POWER_LED_PIN GP25

// =======
// SPI config
#define SPI_DRIVER   SPID0
#define SPI_SCK_PIN  GP2
#define SPI_MOSI_PIN GP3
#define SPI_MISO_PIN GP4

// -- Display
// Rotation (multiple of 90º)
#define LCD_ROTATION 270
#define LCD_CS_PIN   GP5
#define LCD_DC_PIN   GP6
#define LCD_BL_PIN   GP7
#define LCD_RST_PIN  GP8
#define LCD_SPI_MODE 0
#define LCD_SPI_DIV  4
#define QUANTUM_PAINTER_DEBUG

// -- Touch screen
#define TP_CS_PIN  GP9
#define TP_IRQ_PIN GP10

// =======
// RGB
#define DRIVER_LED_TOTAL 16
#define RGB_DI_PIN GP0
#define RGB_DISABLE_WHEN_USB_SUSPENDED
#define RGB_MATRIX_LED_COUNT DRIVER_LED_TOTAL
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 100
#define RGB_MATRIX_STARTUP_MODE RGB_MATRIX_CUSTOM_key_selector_mode
#define WS2812_PIO_USE_PIO1


#define DEBUG_MATRIX_SCAN_RATE


// =======
// SOME MAGIC DOWN HERE
// - Real size and rotation used for init function
#define _LCD_WIDTH  320
#define _LCD_HEIGHT 480
#if !(LCD_ROTATION == 0 || LCD_ROTATION == 90 || LCD_ROTATION == 180 || LCD_ROTATION == 270)
#    error LCD_ROTATION has to be a multiple of 90º between 0º and 360º
#endif

// - Virtual size, used for drawing funcs
#define _LCD_ROTATION (LCD_ROTATION/90)
#if (_LCD_ROTATION % 2 == 0)
#    define LCD_WIDTH  _LCD_WIDTH
#    define LCD_HEIGHT _LCD_HEIGHT
#else
#    define LCD_WIDTH  _LCD_HEIGHT
#    define LCD_HEIGHT _LCD_WIDTH
#endif // LCD_ROTATION

