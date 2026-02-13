/* Copyright 2025 Epomaker
 * Copyright 2025 Guy Sheffer <guysoft@gmail.com>
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

#include "quantum.h"

/* ---------- LED power rail (active-high MOSFET on D0) ---------- */
#ifdef LED_POWER_PIN
void keyboard_pre_init_kb(void) {
    gpio_set_pin_output(LED_POWER_PIN);
    gpio_write_pin_high(LED_POWER_PIN);
    keyboard_pre_init_user();
}

void suspend_power_down_kb(void) {
    gpio_write_pin_low(LED_POWER_PIN);
    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {
    gpio_write_pin_high(LED_POWER_PIN);
    suspend_wakeup_init_user();
}
#endif /* LED_POWER_PIN */

/*
 * FS026 matrix timing overrides — the MCU needs no additional delays
 * beyond what ChibiOS GPIO operations already provide.
 */
void matrix_io_delay(void) {}
void matrix_output_select_delay(void) {}
void matrix_output_unselect_delay(uint8_t line, bool key_pressed) {
    (void)line;
    (void)key_pressed;
}

/* ---------- RGB Matrix LED layout ---------- */
#ifdef RGB_MATRIX_ENABLE
/*
 * g_led_config describes the physical LED positions and their mapping
 * to the key matrix.  There are 82 per-key WS2812 LEDs on the RT82.
 *
 * The three arrays are:
 *   1. matrix_co  – maps [row][col] to LED index (NO_LED = no LED)
 *   2. point      – {x, y} physical position of each LED (0-224 range)
 *   3. flags      – per-LED flags (1 = LED_FLAG_MODIFIER, 4 = LED_FLAG_KEYLIGHT, etc.)
 *                   Using 1 (modifier) for all keys, matching original firmware.
 */
led_config_t g_led_config = { {
    /* matrix_co[6][16] */
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 81, NO_LED },
    { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, NO_LED },
    { 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, NO_LED },
    { 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, NO_LED, 56, 57,  71 },
    { 58, NO_LED, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, NO_LED, 69, 70, NO_LED },
    { 72, 73, 74, NO_LED, NO_LED, 75, NO_LED, NO_LED, NO_LED, 76, 77, NO_LED, NO_LED, 78, 79, 80 }
}, {
    /* point[82] – {x, y} */
    {  0, 10}, { 17, 10}, { 32, 10}, { 47, 10}, { 62, 10}, { 79, 10}, { 94, 10}, {109, 10}, {124, 10}, {141, 10}, {156, 10}, {171, 10}, {186, 10}, {206, 10},
    {  0, 20}, { 15, 20}, { 30, 20}, { 45, 20}, { 60, 20}, { 75, 20}, { 90, 20}, {105, 20}, {120, 20}, {135, 20}, {150, 20}, {165, 20}, {180, 20}, {200, 20},             {224, 20},
    {  4, 30}, { 20, 30}, { 35, 30}, { 50, 30}, { 65, 30}, { 80, 30}, { 95, 30}, {110, 30}, {125, 30}, {140, 30}, {155, 30}, {170, 30}, {185, 30},             {204, 30}, {224, 30},
    {  6, 40}, { 24, 40}, { 39, 40}, { 54, 40}, { 69, 40}, { 84, 40}, { 99, 40}, {114, 40}, {129, 40}, {144, 40}, {159, 40}, {174, 40},             {199, 40},             {224, 40},
    {  8, 50},             { 28, 50}, { 43, 50}, { 58, 50}, { 73, 50}, { 88, 50}, {103, 50}, {118, 50}, {133, 50}, {148, 50}, {163, 50},             {183, 50}, {203, 50}, {224, 50},
    {  0, 60}, { 20, 60}, { 40, 60},                        { 90, 60},                                  {145, 60}, {165, 60},                       {188, 60}, {203, 60}, {224, 60}, {224, 10}
}, {
    /* flags[82] – all key LEDs */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1,    1,
    1,    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1, 1, 1,
    1, 1, 1,       1,          1, 1,       1, 1, 1, 1
} };

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }
    return true;
}
#endif /* RGB_MATRIX_ENABLE */
