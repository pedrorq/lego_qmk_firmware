// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "config_common.h"

// =======
// Debugging
// #define CUSTOM_SPI_DEBUG
// #define DEBUG_MATRIX_SCAN_RATE
#define INIT_DELAY 3000
// #define QUANTUM_PAINTER_DEBUG
// #define SIPO_PINS_DEBUG
// #define TOUCH_SCREEN_DEBUG

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
#define SPI_MODE 0
#define SPI_DIV 0
#define SPI_DIV_9486 8 //cant handle faster speed

// -- Register's data
#define SIPO_SPI_DRIVER SPID1
#define SIPO_SPI_SCK_PIN GP10
#define SIPO_SPI_MOSI_PIN GP11
#define SIPO_SPI_MISO_PIN GP12
#define SIPO_CS_PIN GP13
#define SIPO_SPI_MODE 0
#define SIPO_SPI_DIV  0

#define SIPO_PINS 9

// -- Displays
#define IL91874_ROTATION 0
#define ILI9163_ROTATION 0
#define ILI9341_ROTATION 2
#define ILI9486_ROTATION 1
#define SSD1680_ROTATION 0

#define ILI9341_TOUCH_IRQ_PIN GP15
#define ILI9486_TOUCH_IRQ_PIN GP9

// If "virtual" pins aren't enabled, use real pins
#if defined(SIPO_PINS_ENABLE)
#    define DUMMY_PIN GP1
#elif defined(QUANTUM_PAINTER_ENABLE)
#    error "Hardware design only supports SIPO output to control displays"
#endif // SIPO_PINS_ENABLE

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
