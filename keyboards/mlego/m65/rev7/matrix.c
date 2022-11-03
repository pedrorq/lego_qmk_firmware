#include "quantum.h"

#include "wait.h"
#include "spi_master.h"

#ifdef CONSOLE_ENABLE
#include "print.h"
#endif

static const uint16_t col_values[MATRIX_COLS] = COLS;
static const uint8_t row_pins[MATRIX_ROWS] = ROWS;

static inline uint8_t read_rows(void) {
  uint8_t r = readPin(row_pins[0]);

  for (uint8_t row = 1; row < MATRIX_ROWS; row++) {
    r |= (readPin(row_pins[row]) << row);
   }
  return r;
}

static inline void shift_out(uint16_t value) {

  uint8_t message[2]  = {(value >> 8) & 0xFF ,(uint8_t)(value & 0xFF) };

  spi_start(SPI_LATCH_PIN, true, 1, SPI_DIVISOR);
  spi_transmit(message,2);
#ifdef CONSOLE_ENABLE
//  bool ans = spi_start(SPI_LATCH_PIN, true, 1, SPI_DIVISOR);
//  spi_status_t info = spi_transmit(message,2);
//  uprintf("column: %u, status_start: %u, status_transmit: %u, m1: %u and m2: %u\n", value, ans, info, message[0], message[1]);
#endif
  spi_stop();

  matrix_io_delay();
}

static inline void select_col(uint8_t col) {
    shift_out(col_values[col]);
}

void matrix_init_custom(void) {

#ifdef CONSOLE_ENABLE
  wait_ms(3000);
  printf("set rows\n");
#endif
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    setPinInputLow(row_pins[row]);
  }
#ifdef CONSOLE_ENABLE
  printf("done rows\n");
#endif

#ifdef CONSOLE_ENABLE
    printf("set spi\n");
#endif
  wait_ms(100);
  spi_init();
  wait_ms(100);
#ifdef CONSOLE_ENABLE
    printf("done spi\n");
#endif
  writePinHigh(SPI_LATCH_PIN); // should be high when using SPI?
  setPinOutput(SPI_LATCH_PIN);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        select_col(col);
        matrix_io_delay();

        uint8_t rows = read_rows();
#ifdef CONSOLE_ENABLE
//    uprintf("rows: %u\n",rows);
#endif
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            bool prev_bit = (current_matrix[row] & (matrix_row_t)(1 << col))? 1 : 0;
            bool curr_bit = (rows & (1 << row))? 1 : 0;
            if (prev_bit != curr_bit) {
                current_matrix[row] = current_matrix[row] ^ (1 << col);
                changed = true;
            }
        }
    }
    return changed;
}
