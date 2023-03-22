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
#include "tipro.h"

#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define LED_DELAY 100

uint8_t PROGMEM led_s = 0;

void pulseHighLow(pin_t pin){
  writePinHigh(pin);
  wait_ms(LED_DELAY);
  writePinLow(pin);

}

void shiftOutShort(pin_t dataPin, pin_t clockPin, uint8_t val){
  for (uint8_t i = 0; i < 8; i++)  {
    writePin(dataPin,!!(led_s & (1 << i)));
    pulseHighLow(clockPin);
  }
}

void leds_off(void) {

  led_s = 0;
  writePinLow(SR_LATCH_PIN);
  shiftOutShort(SR_DATA_PIN, SR_CLOCK_PIN, led_s);
  writePinHigh(SR_LATCH_PIN);
}

void led_on(uint8_t led) {

  led_s |= led;
  writePinLow(SR_LATCH_PIN);
  shiftOutShort(SR_DATA_PIN, SR_CLOCK_PIN, led_s);
  writePinHigh(SR_LATCH_PIN);

}

void led_toggle(uint8_t led) {

  led_s ^= led;
  writePinLow(SR_LATCH_PIN);
  shiftOutShort(SR_DATA_PIN, SR_CLOCK_PIN, led_s);
  writePinHigh(SR_LATCH_PIN);

}

void matrix_init_kb(void) {
  setup_leds();
  leds_off();
  count_leds();
}

void setup_leds(void) {

  writePinLow(SR_LATCH_PIN);
  setPinOutput(SR_LATCH_PIN);

  writePinLow(SR_DATA_PIN);
  setPinOutput(SR_DATA_PIN);

  writePinLow(SR_CLOCK_PIN);
  setPinOutput(SR_CLOCK_PIN);
}

void count_leds(void){
  uint8_t leds = 0;
  for (uint8_t i=0; i<8; i++ ) {
    bitSet(leds,i);
    led_on(leds);
    wait_ms(LED_DELAY);
  }
  leds_off();
}
