// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

// =======
// Multi-SPI driver config
#if defined(SIPO_SPI_DRIVER)
#    define SPI_DRIVERS SPI_DRIVER, SIPO_SPI_DRIVER
#else
#    define SPI_DRIVERS SPI_DRIVER
#endif

#if defined(SIPO_SPI_SCK_PIN)
#    define SPI_SCK_PINS SPI_SCK_PIN, SIPO_SPI_SCK_PIN
#else
#    define SPI_SCK_PINS SPI_SCK_PIN
#endif

#if defined(SIPO_SPI_MOSI_PIN)
#    define SPI_MOSI_PINS SPI_MOSI_PIN, SIPO_SPI_MOSI_PIN
#else
#    define SPI_MOSI_PINS SPI_MOSI_PIN
#endif

#if defined(SIPO_SPI_MISO_PIN)
#    define SPI_MISO_PINS SPI_MISO_PIN, SIPO_SPI_MISO_PIN
#else
#    define SPI_MISO_PINS SPI_MISO_PIN
#endif

// =======
// Quantum Painter config
// - Real size, used for init function
#define _IL91874_WIDTH 176
#define _IL91874_HEIGHT 264
#define _ILI9163_WIDTH 129
#define _ILI9163_HEIGHT 128
#define _ILI9341_WIDTH 240
#define _ILI9341_HEIGHT 320
#define _ILI9486_WIDTH 320
#define _ILI9486_HEIGHT 480

#define SPI_ROTATION 0
#define _SSD1680_WIDTH 128
#define _SSD1680_HEIGHT 250

// - Check rotation
#if !(ILI9163_ROTATION == 0 || ILI9163_ROTATION == 1 || ILI9163_ROTATION == 2 || ILI9163_ROTATION == 3)
#    error ILI9163_ROTATION has to be within 0-3 (both included)
#endif
#if !(ILI9341_ROTATION == 0 || ILI9341_ROTATION == 1 || ILI9341_ROTATION == 2 || ILI9341_ROTATION == 3)
#    error ILI9341_ROTATION has to be within 0-3 (both included)
#endif
#if !(ILI9486_ROTATION == 0 || ILI9486_ROTATION == 1 || ILI9486_ROTATION == 2 || ILI9486_ROTATION == 3)
#    error ILI9486_ROTATION has to be within 0-3 (both included)
#endif
#if !(IL91874_ROTATION == 0 || IL91874_ROTATION == 1 || IL91874_ROTATION == 2 || IL91874_ROTATION == 3)
#    error IL91874_ROTATION has to be within 0-3 (both included)
#endif
#if !(SSD1680_ROTATION == 0 || SSD1680_ROTATION == 1 || SSD1680_ROTATION == 2 || SSD1680_ROTATION == 3)
#    error SSD1680_ROTATION has to be within 0-3 (both included)
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

#if (ILI9486_ROTATION % 2 == 0)
#    define ILI9486_WIDTH  _ILI9486_WIDTH
#    define ILI9486_HEIGHT _ILI9486_HEIGHT
#else
#    define ILI9486_WIDTH  _ILI9486_HEIGHT
#    define ILI9486_HEIGHT _ILI9486_WIDTH
#endif

#if (IL91874_ROTATION % 2 == 0)
#    define IL91874_WIDTH  _IL91874_WIDTH
#    define IL91874_HEIGHT _IL91874_HEIGHT
#else
#    define IL91874_WIDTH  _IL91874_HEIGHT
#    define IL91874_HEIGHT _IL91874_WIDTH
#endif

#if (SSD1680_ROTATION % 2 == 0)
#    define SSD1680_WIDTH  _SSD1680_WIDTH
#    define SSD1680_HEIGHT _SSD1680_HEIGHT
#else
#    define SSD1680_WIDTH  _SSD1680_HEIGHT
#    define SSD1680_HEIGHT _SSD1680_WIDTH
#endif

// =======
// - Default values
#if !defined(INIT_DELAY)
#    define INIT_DELAY 5
#endif // INIT_DELAY
