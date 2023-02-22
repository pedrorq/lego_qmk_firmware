// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// =======
// Multi-SPI driver config
#define SPI_DRIVERS SCREENS_SPI_DRIVER, REGISTERS_SPI_DRIVER
#define SPI_SCK_PINS SCREENS_SCK_PIN, REGISTERS_SCK_PIN
#define SPI_MOSI_PINS SCREENS_MOSI_PIN, REGISTERS_MOSI_PIN
#define SPI_MISO_PINS SCREENS_MISO_PIN, REGISTERS_MISO_PIN

// =======
// Quantum Painter config
// - Real size, used for init function
#define _IL91874_WIDTH 176
#define _IL91874_HEIGHT 264
#define _ILI9163_WIDTH 129
#define _ILI9163_HEIGHT 128
#define _ILI9341_WIDTH 240
#define _ILI9341_HEIGHT 320

// - Check rotation
#if !(ILI9163_ROTATION == 0 || ILI9163_ROTATION == 1 || ILI9163_ROTATION == 2 || ILI9163_ROTATION == 3)
#    error ILI9163_ROTATION has to be within 0-3 (both included)
#endif
#if !(ILI9341_ROTATION == 0 || ILI9341_ROTATION == 1 || ILI9341_ROTATION == 2 || ILI9341_ROTATION == 3)
#    error ILI9341_ROTATION has to be within 0-3 (both included)
#endif
#if !(IL91874_ROTATION == 0 || IL91874_ROTATION == 1 || IL91874_ROTATION == 2 || IL91874_ROTATION == 3)
#    error IL91874_ROTATION has to be within 0-3 (both included)
#endif

// - Virtual size, used for drawing funcs
#if (ILI9163_ROTATION % 2 == 0)
#    define ILI9163_WIDTH  _ILI9163_WIDTH
#    define ILI9163_HEIGHT _ILI9163_HEIGHT
#else
#    define ILI9163_WIDTH  _ILI9163_HEIGHT
#    define ILI9163_HEIGHT _ILI9163_WIDTH
#endif

#if (ILI9341_ROTATION % 2 == 0)
#    define ILI9341_WIDTH  _ILI9341_WIDTH
#    define ILI9341_HEIGHT _ILI9341_HEIGHT
#else
#    define ILI9341_WIDTH  _ILI9341_HEIGHT
#    define ILI9341_HEIGHT _ILI9341_WIDTH
#endif

#if (IL91874_ROTATION % 2 == 0)
#    define IL91874_WIDTH  _IL91874_WIDTH
#    define IL91874_HEIGHT _IL91874_HEIGHT
#else
#    define IL91874_WIDTH  _IL91874_HEIGHT
#    define IL91874_HEIGHT _IL91874_WIDTH
#endif

// =======
// - Default values
#if !defined(INIT_DELAY)
#    define INIT_DELAY 5
#endif // INIT_DELAY
