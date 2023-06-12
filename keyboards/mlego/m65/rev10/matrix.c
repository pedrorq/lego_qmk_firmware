// Copyright 2020-2023 alin m elena (@alinelena, @drFaustroll)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"

#include "wait.h"
#include "spi_master.h"

#ifdef CONSOLE_ENABLE
#include "print.h"
#endif

static const uint16_t col_values[MATRIX_COLS] = COLS;
static const pin_t rows_values[MATRIX_ROWS] = ROWS;
static const int msize = MATRIX_ROWS * sizeof(matrix_row_t);
static matrix_row_t prev_matrix[MATRIX_ROWS];

static inline uint8_t read_rows(void) {

    writePinLow(SPI_CS_PIN);
 //   matrix_output_select_delay();

    spi_status_t read_result = spi_read();
    if (read_result >= 0) {
    //   if (read_result > 0) {
    //   uprintf("row code: %u \n",read_result);
    //   }
       return (uint8_t) read_result;
     } else{
       return 0;
     }
}

static inline void shift_out(uint16_t value) {

  uint8_t message[2]  = {(value >> 8) & 0xFF ,(uint8_t)(value & 0xFF) };

  writePinLow(SPI_CS_PIN);
  spi_transmit(message,2);
  writePinHigh(SPI_CS_PIN);
  matrix_output_select_delay();


}

void matrix_init_custom(void) {

#ifdef CONSOLE_ENABLE
  wait_ms(3000);
#endif
  matrix_io_delay();
  spi_init();
  matrix_io_delay();

  setPinOutput(SPI_CS_PIN);
  writePinHigh(SPI_CS_PIN);
  matrix_io_delay();
  spi_start(SPI_CS_PIN, SPI_LSBFIRST, SPI_MODE, SPI_DIVISOR);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {

    uint8_t crow;
    memset(current_matrix, 0, msize);


    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
#ifdef CONSOLE_ENABLE
 //   uprintf("col: %u, code: %u \n",col,col_values[col]);
#endif
        shift_out(col_values[col]);

        uint8_t rows = read_rows();
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
             crow = rows_values[row];
             current_matrix[row] |= (((rows & (1 << crow))? 1 : 0) << col);
            }
        }
    bool changed = (memcmp(current_matrix, prev_matrix, msize) != 0);
    memcpy(prev_matrix, current_matrix, msize);
    return changed;
}
