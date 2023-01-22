// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

void sram_init(painter_device_t device);

bool sram_flush(painter_device_t device);
bool sram_pixdata(painter_device_t device, const void *pixel_data, uint32_t native_pixel_count);
