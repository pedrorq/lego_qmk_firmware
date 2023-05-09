/*
Copyright 2021-2022 Alin M Elena <alinm.elena@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define LED_LWR_PIN C15
#define LED_RSE_PIN C13


#ifdef OLED_ENABLE
#define I2C_DRIVER I2CD1
#define USE_I2C2_100KHZ
#    define OLED_DISPLAY_128X32
#define I2C1_SCL_PIN        A15
#define I2C1_SCL_PAL_MODE 4
#define I2C1_SDA_PIN        B7
#define I2C1_SDA_PAL_MODE 4

#define OLED_BRIGHTNESS 128
#define OLED_FONT_H "keyboards/mlego/m65/lib/glcdfont.c"
#if defined(USE_I2C1_400KHZ) || defined(USE_I2C2_400KHZ) || defined(USE_I2C3_400KHZ)
#    define I2C1_TIMINGR_PRESC 0U
#    define I2C1_TIMINGR_SCLDEL 15U
#    define I2C1_TIMINGR_SDADEL 0U
#    define I2C1_TIMINGR_SCLH 123U
#    define I2C1_TIMINGR_SCLL 255U
#elif defined(USE_I2C1_100KHZ) || defined(USE_I2C2_100KHZ) || defined(USE_I2C3_100KHZ)
#    define I2C1_TIMINGR_PRESC 3U
#    define I2C1_TIMINGR_SCLDEL 9U
#    define I2C1_TIMINGR_SDADEL 0U
#    define I2C1_TIMINGR_SCLH 157U
#    define I2C1_TIMINGR_SCLL 236U
#endif
#endif

#define DEBUG_MATRIX_SCAN_RATE
