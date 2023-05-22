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

#include "quantum.h"

//#ifdef UNICODEMAP_ENABLE
enum unicode_names { la = SAFE_RANGE, lA, lb, lB, lc, lC, ld, lD, le, lE, lf, lF, lg, lG, lh, lH, li, lI, lj, lJ, lk, lK, ll, lL, lm, lM, ln, lN, lo, lO, lp, lP, lq, lQ, lr, lR, ls, lS, lt, lT, lu, lU, lv, lV, lw, lW, lx, lX, ly, lY, lz, lZ, lc1, lC1, lp1, lP1, lq1, lQ1, ll1, lL1, lk1, lK1, lbo1, lbo2, lbc1, lbc2, ls1, ls2, ra, rA, ra1, rA1, rc, rC, ri, rI, rs, rS, rt, rT, ro, rO, ra2, rA2, ra3, rA3, rx, rX, rv, rV, rb, rB, rn, rN };
//#endif

enum layer_names {
    _QW = 0,
    _LWR,
    _RSE,
    _ADJ
};

void toggle_leds(const bool, const bool);
void set_led_toggle(const uint8_t, const bool);

static inline void init_lwr_rse_led(void) {
#ifdef LED_LWR_PIN
    setPinOutput(LED_LWR_PIN);
    writePin(LED_LWR_PIN, false);
    wait_ms(30);
#endif

#ifdef LED_RSE_PIN
    setPinOutput(LED_RSE_PIN);
    writePin(LED_RSE_PIN, false);
    wait_ms(30);
#endif
}

static inline void led_lwr(const bool on) {
#ifdef LED_LWR_PIN
    if ((PRODUCT_ID == 0x6069) || (PRODUCT_ID == 0x6070)) {
        writePin(LED_LWR_PIN, !on);
    }else{
        writePin(LED_LWR_PIN, on);
    }
#endif
}

static inline void led_rse(const bool on) {
#ifdef LED_RSE_PIN
    if ((PRODUCT_ID == 0x6069) || (PRODUCT_ID == 0x6070)) {
        writePin(LED_RSE_PIN, !on);
    }else{
        writePin(LED_RSE_PIN, on);
    }
#endif
}
