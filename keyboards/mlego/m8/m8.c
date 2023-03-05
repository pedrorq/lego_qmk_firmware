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

#include "m8.h"

#if defined(QUANTUM_PAINTER_ENABLE)
#include "color.h"
#include "graphics.h"
#include "qp.h"
#include "qp_eink_panel.h"
#include "qp_surface.h"
#include "version.h"
painter_device_t ssd1680;
uint8_t ssd1680_buffer[EINK_BYTES_REQD(SSD1680_WIDTH, SSD1680_HEIGHT)];

uint32_t flush_display(uint32_t trigger_time, void *device) {
    qp_flush((painter_device_t *)device);
    return 0;
}
#if defined(CUSTOM_EEPROM_ENABLE)
#    include "custom_eeprom.h"
#endif // CUSTOM_EEPROM_ENABLE
#endif // QUANTUM_PAINTER_ENABLE


// let us assume we start with both layers off
static bool toggle_lwr = false;
static bool toggle_rse = false;

#ifdef RGBLIGHT_ENABLE

const rgblight_segment_t PROGMEM my_qwerty_layer[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLED_NUM, HSV_PURPLE});
const rgblight_segment_t PROGMEM my_lwr_layer[]    = RGBLIGHT_LAYER_SEGMENTS({0, RGBLED_NUM, HSV_CYAN});
const rgblight_segment_t PROGMEM my_rse_layer[]    = RGBLIGHT_LAYER_SEGMENTS({0, RGBLED_NUM, HSV_RED});
const rgblight_segment_t PROGMEM my_adj_layer[]    = RGBLIGHT_LAYER_SEGMENTS({0, RGBLED_NUM, HSV_GREEN});

const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(my_qwerty_layer, my_lwr_layer, my_rse_layer, my_adj_layer);

#endif


#ifdef RGBLIGHT_ENABLE

void set_rgb_layers(layer_state_t state){

    rgblight_set_layer_state(0, layer_state_cmp(state, _QW));
    rgblight_set_layer_state(1, layer_state_cmp(state, _LWR));
    rgblight_set_layer_state(2, layer_state_cmp(state, _RSE));
    rgblight_set_layer_state(3, layer_state_cmp(state, _ADJ));

}

void set_default_rgb_layers(layer_state_t state){
    rgblight_set_layer_state(0, layer_state_cmp(state, _QW));
}

const rgblight_segment_t * const* my_rgb(void){
    return my_rgb_layers;
}

#endif

void set_led_toggle(const uint8_t layer, const bool state){

    switch (layer) {
        case _LWR:
          toggle_lwr = state;
          break;
        case _RSE:
          toggle_rse = state;
          break;
        default:
          break;
    }
}

void toggle_leds(void){

    led_lwr(toggle_lwr);
    led_rse(toggle_rse);
    led_t led_state = host_keyboard_led_state();
    led_caps(led_state.caps_lock);
    if (layer_state_is(_ADJ)) {
        led_lwr(true);
        led_rse(true);
    }

}

#if defined(QUANTUM_PAINTER_ENABLE)

char build_date[] = QMK_BUILDDATE;
char commit_hash[] = QMK_GIT_HASH;

#endif

uint32_t deferred_init(uint32_t trigger_time, void *cb_arg) {
    dprint("---------- Init phase ----------\n");

#if defined(QUANTUM_PAINTER_ENABLE)


    wait_ms(1500); //Let screens draw some power
    load_qp_resources();


    ssd1680 = qp_ssd1680_make_spi_device(_SSD1680_WIDTH, _SSD1680_HEIGHT, EINK_CS_PIN, EINK_DC_PIN, EINK_RST_PIN, SPI_DIVISOR, SPI_MODE, (void *)ssd1680_buffer);
    load_display(ssd1680);
    qp_init(ssd1680, SSD1680_ROTATION);


    // show EEPROM state (enable features)
#    if defined(CUSTOM_EEPROM_ENABLE)
    // enabled features based on `#define`s on current firmware
    uint32_t current_config = custom_eeprom_generate();

    // if any change has ocurred, we could run some logic
    if (current_config != eeconfig_read_user()) {
        dprintf("EEPROM config has changed\n");
        eeconfig_update_user(current_config);
        custom_eeprom_draw_config((void *)ssd1680);
    }

#    endif // CUSTOM_EEPROM_ENABLE

    dprint("Quantum painter devices initialised\n");



    qp_rect(ssd1680, 0, 0, SSD1680_WIDTH, SSD1680_HEIGHT, HSV_WHITE, true);
    qp_rect(ssd1680, 0, 0, SSD1680_WIDTH/2, SSD1680_HEIGHT/2, HSV_BLACK, true);
    qp_drawimage_recolor(ssd1680, 0, SSD1680_HEIGHT/2-70, qp_images[3], HSV_BLACK, HSV_WHITE);
    qp_drawimage_recolor(ssd1680, 40, 110, qp_images[8], HSV_BLACK, HSV_WHITE);
    qp_drawimage_recolor(ssd1680, 70, SSD1680_HEIGHT/2-70, qp_images[7], HSV_BLACK, HSV_WHITE);
    qp_rect(ssd1680, 0, 0, SSD1680_WIDTH-7, SSD1680_HEIGHT-1, HSV_BLACK, false);
    char hello[] = "Hello World!!!";
    int16_t               hello_width = qp_textwidth(qp_fonts[0], hello);
    qp_drawtext_recolor(ssd1680, SSD1680_WIDTH-hello_width-10, 5, qp_fonts[0],hello,HSV_BLACK,HSV_WHITE);
    int16_t               hash_width = qp_textwidth(qp_fonts[0], commit_hash);
    qp_drawtext_recolor(ssd1680, SSD1680_WIDTH-hash_width-10, SSD1680_HEIGHT-1.25*qp_fonts[0]->line_height, qp_fonts[0], commit_hash, HSV_BLACK, HSV_WHITE);
    qp_drawtext_recolor(ssd1680, SSD1680_WIDTH-hash_width-10, 5+qp_fonts[0]->line_height, qp_fonts[0], commit_hash, HSV_BLACK, HSV_WHITE);
    int16_t               build_width = qp_textwidth(qp_fonts[1], build_date);
    qp_drawtext_recolor(ssd1680, SSD1680_WIDTH-build_width-10, SSD1680_HEIGHT-2.5*qp_fonts[1]->line_height,qp_fonts[1], build_date, HSV_BLACK, HSV_WHITE);
    qp_drawtext_recolor(ssd1680, SSD1680_WIDTH-build_width-10, 5+2.25*qp_fonts[0]->line_height,qp_fonts[1], build_date, HSV_BLACK, HSV_WHITE);
    eink_panel_dc_reset_painter_device_t *eink = (eink_panel_dc_reset_painter_device_t *)ssd1680;
    defer_exec(eink->timeout, flush_display, (void *)ssd1680);

    dprint("Quantum painter ready\n");
#endif // QUANTUM_PAINTER_ENABLE


    dprint("\n---------- User code ----------\n");

    // =======
    // Call user code
    keyboard_post_init_user();

    return 0;
}

#if defined(QUANTUM_PAINTER_ENABLE)
void keyboard_post_init_kb(void) {
#ifdef CONSOLE_ENABLE
  debug_enable = true;
  debug_matrix = true;
  debug_keyboard = true;
#endif

    defer_exec(INIT_DELAY, deferred_init, NULL);

}
#endif


