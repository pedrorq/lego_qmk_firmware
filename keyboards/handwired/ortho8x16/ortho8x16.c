/* Copyright 2021-2022 alin elena <alin@elena.space>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ortho8x16.h"

#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define LED_DELAY 500

void pulseHighLow(pin_t pin){
  writePinHigh(pin);
  wait_ms(LED_DELAY);
  writePinLow(pin);

}

void shiftOutShort(pin_t dataPin, pin_t clockPin, uint8_t val){

  for (uint8_t i = 0; i < 7; i++)  {
    writePin(dataPin,!!(val & (1 << i)));
    wait_ms(LED_DELAY);
    pulseHighLow(clockPin);
  }
}

void leds_off(void) {

  uint8_t l = 0;

  leds_on(l);
}

void leds_on(uint8_t leds) {

  writePinLow(SR_LATCH_PIN);
  shiftOutShort(SR_DATA_PIN, SR_CLOCK_PIN, leds);
  writePinHigh(SR_LATCH_PIN);

}

void matrix_init_kb(void) {
  setup_leds();
}

void setup_leds(void) {

  writePinLow(SR_LATCH_PIN);
  setPinOutput(SR_LATCH_PIN);

  writePinLow(SR_OE_PIN);
  setPinOutput(SR_OE_PIN);

  writePinLow(SR_DATA_PIN);
  setPinOutput(SR_DATA_PIN);

  writePinLow(SR_CLOCK_PIN);
  setPinOutput(SR_CLOCK_PIN);

  //writePinLow(SR_OE_PIN);
  writePinHigh(SR_OE_PIN);

  uint8_t leds = 0;
  for (uint8_t i=0; i<7; i++ ) {
    bitSet(leds,i);
    leds_on(leds);
    wait_ms(LED_DELAY);
  }
  leds = 1;
  leds_on(leds);
  wait_ms(LED_DELAY);

}
