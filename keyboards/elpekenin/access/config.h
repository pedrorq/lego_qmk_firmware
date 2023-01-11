// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "config_common.h"

// =======
// Debugging
#define QUANTUM_PAINTER_DEBUG
// #define DEBUG_MATRIX_SCAN_RATE
#define INIT_DELAY 3000
#define TOUCH_SCREEN_DEBUG

// =======
// Power indicator
#define POWER_LED_PIN GP25

// =======
// SPI config
#define SPI_COUNT 2
// -- Screens' data
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP2
#define SPI_MOSI_PIN GP3
#define SPI_MISO_PIN GP4

#define SPI_DC_PIN GP6
#define SPI_MODE 0
#define SPI_DIV 16

// -- Register's data
#define REGISTER_SPI_DRIVER SPID1
#define REGISTER_SPI_SCK_PIN GP10
#define REGISTER_SPI_MOSI_PIN GP11
#define REGISTER_SPI_MISO_PIN GP12

#define REGISTER_CS_PIN GP13
#define REGISTER_SPI_MODE 3
#define REGISTER_SPI_DIV 32

#define REGISTER_PINS 24

// -- Displays
#define IL91874_ROTATION 0
#define ILI9163_ROTATION 0
#define ILI9341_ROTATION 2
#define ILI9486_ROTATION 1
#define SSD1680_ROTATION 0

#define ILI9341_TOUCH_IRQ_PIN GP15
#define ILI9486_TOUCH_IRQ_PIN GP9

// If "virtual" pins aren't enabled, use real pins
#ifndef SIPO_PINS
#    define IL91874_CS_PIN GP18
#    define IL91874_RST_PIN GP1

#    define ILI9163_CS_PIN GP10
#    define ILI9163_RST_PIN GP11

#    define ILI9341_CS_PIN GP12
#    define ILI9341_RST_PIN GP13
#    define ILI9341_TOUCH_CS_PIN GP14

#    define ILI9486_CS_PIN GP5
#    define ILI9486_RST_PIN GP7
#    define ILI9486_TOUCH_CS_PIN GP8

#    define SSD1680_CS_PIN GP18
#    define SSD1680_RST_PIN GP1
#    define SCREEN_SPI_DC_PIN SPI_DC_PIN
#endif // SIPO_PINS

// =======
// RGB
#define RGB_DI_PIN GP0
#define RGB_DISABLE_WHEN_USB_SUSPENDED
#define RGB_MATRIX_LED_COUNT 16
#define WS2812_PIO_USE_PIO1

// =======
// Pointing device
#define ANALOG_JOYSTICK_CLICK_PIN  GP15
#define ANALOG_JOYSTICK_X_AXIS_PIN GP28
#define ANALOG_JOYSTICK_Y_AXIS_PIN GP27

// =======
// Audio
#define AUDIO_PIN GP11
#define AUDIO_PWM_DRIVER PWMD5
#define AUDIO_PWM_CHANNEL RP2040_PWM_CHANNEL_B
#define AUDIO_ENABLE_TONE_MULTIPLEXING
#define AUDIO_TONE_MULTIPLEXING_RATE_DEFAULT 10
