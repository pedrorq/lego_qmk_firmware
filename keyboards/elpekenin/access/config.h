// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "config_common.h"

// =======
// Debugging
#define QUANTUM_PAINTER_DEBUG
// #define DEBUG_MATRIX_SCAN_RATE
#define INIT_DELAY 2000

// =======
// Painter
#define QUANTUM_PAINTER_NUM_IMAGES 16

// =======
// Power indicator
#define POWER_LED_PIN GP25

// =======
// SPI config
#define SPI_DRIVER   SPID0
#define SPI_SCK_PIN  GP2
#define SPI_MOSI_PIN GP3
#define SPI_MISO_PIN GP4
#define SPI_MODE 0
#define SPI_DIV  16

// -- Display
// Rotation (multiple of 90º)
#define SCREEN_ROTATION 270
#define LCD_CS_PIN   GP5
#define LCD_DC_PIN   GP6
#define LCD_BL_PIN   GP7
#define LCD_RST_PIN  GP8

// -- Touch screen
#define TOUCH_CS_PIN  GP9
#define TOUCH_IRQ_PIN GP10

// =======
// RGB
#define ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
#define RGB_DI_PIN GP0
#define RGB_DISABLE_WHEN_USB_SUSPENDED
#define RGB_MATRIX_LED_COUNT 16
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 10
#define RGB_MATRIX_STARTUP_VAL RGB_MATRIX_MAXIMUM_BRIGHTNESS
#if defined(ONE_HAND_MODE)
#    define RGB_MATRIX_STARTUP_MODE RGB_MATRIX_CUSTOM_key_selector_mode
#else
#    define RGB_MATRIX_STARTUP_MODE RGB_MATRIX_CYCLE_LEFT_RIGHT
#endif   // ONE_HAND_MODE
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
#define AUDIO_INIT_DELAY 0
#if defined(AUDIO_ENABLE)
#    define STARTUP_SONG SONG(ONE_UP_SOUND)
#    define GOODBYE_SONG SONG(COIN_SOUND)
#endif // AUDIO_ENABLE

// =======
// SOME MAGIC DOWN HERE
// - Real size and rotation used for init function
#define _SCREEN_WIDTH  320
#define _SCREEN_HEIGHT 480
#if !(SCREEN_ROTATION == 0 || SCREEN_ROTATION == 90 || SCREEN_ROTATION == 180 || SCREEN_ROTATION == 270)
#    error SCREEN_ROTATION has to be a multiple of 90º between 0º and 360º
#endif

// - Virtual size, used for drawing funcs
#define _SCREEN_ROTATION (SCREEN_ROTATION/90)
#if (_SCREEN_ROTATION % 2 == 0)
#    define SCREEN_WIDTH  _SCREEN_WIDTH
#    define SCREEN_HEIGHT _SCREEN_HEIGHT
#else
#    define SCREEN_WIDTH  _SCREEN_HEIGHT
#    define SCREEN_HEIGHT _SCREEN_WIDTH
#endif // SCREEN_ROTATION

// - Default values
#if !defined(INIT_DELAY)
#    define INIT_DELAY 5
#endif // INIT_DELAY
