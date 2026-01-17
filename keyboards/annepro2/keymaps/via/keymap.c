/* Copyright 2021 OpenAnnePro community
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

#include QMK_KEYBOARD_H
#include "ap2_led.h"

// Manual one-shot layer tracking (since built-in OSL doesn't work well on AP2)
static bool osl_fn1_active = false;

// Helper to check if a keycode is a modifier
static bool is_modifier_keycode(uint16_t keycode) {
    return (keycode >= KC_LCTL && keycode <= KC_RGUI) ||
           (keycode & 0xFF00) == QK_MODS ||  // Mod-tap keys when held
           keycode == KC_LSFT || keycode == KC_RSFT ||
           keycode == KC_LCTL || keycode == KC_RCTL ||
           keycode == KC_LALT || keycode == KC_RALT ||
           keycode == KC_LGUI || keycode == KC_RGUI;
}

enum anne_pro_layers {
    BASE,
    FN1,
    FN2,
};

// Custom keycodes
enum custom_keycodes {
    OSL_RALT = SAFE_RANGE,  // Right Alt on hold, One-Shot FN1 on tap
};

// Mod-tap placeholder for OSL_RALT (Alt on hold, custom on tap)
#define MT_OSL_RALT RALT_T(KC_NO)

// Color definitions
#define COLOR_RED    ((ap2_led_t){.p.red = 0xff, .p.green = 0x00, .p.blue = 0x00, .p.alpha = 0xff})
#define COLOR_BLUE   ((ap2_led_t){.p.red = 0x00, .p.green = 0x00, .p.blue = 0xff, .p.alpha = 0xff})
#define COLOR_YELLOW ((ap2_led_t){.p.red = 0xff, .p.green = 0xff, .p.blue = 0x00, .p.alpha = 0xff})
#define COLOR_ORANGE ((ap2_led_t){.p.red = 0xff, .p.green = 0x80, .p.blue = 0x00, .p.alpha = 0xff})
#define COLOR_CYAN   ((ap2_led_t){.p.red = 0x00, .p.green = 0xff, .p.blue = 0xff, .p.alpha = 0xff})
#define COLOR_PURPLE ((ap2_led_t){.p.red = 0xff, .p.green = 0x00, .p.blue = 0xff, .p.alpha = 0xff})
#define COLOR_GREEN  ((ap2_led_t){.p.red = 0x00, .p.green = 0xff, .p.blue = 0x00, .p.alpha = 0xff})
#define COLOR_LIGHT_BLUE ((ap2_led_t){.p.red = 0x00, .p.green = 0x80, .p.blue = 0xff, .p.alpha = 0xff})

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
 [BASE] = LAYOUT_60_ansi( /* Base */
    KC_ESC,           KC_1,    KC_2,    KC_3, KC_4, KC_5, KC_6,   KC_7, KC_8, KC_9,    KC_0,             KC_MINS,          KC_EQL,        KC_BSPC,
    KC_TAB,           KC_Q,    KC_W,    KC_E, KC_R, KC_T, KC_Y,   KC_U, KC_I, KC_O,    KC_P,             KC_LBRC,          KC_RBRC,       KC_BSLS,
    LT(FN1, KC_CAPS), KC_A,    KC_S,    KC_D, KC_F, KC_G, KC_H,   KC_J, KC_K, KC_L,    KC_SCLN,          KC_QUOT,          KC_ENT,
    KC_LSFT,                   KC_Z,    KC_X, KC_C, KC_V, KC_B,   KC_N, KC_M, KC_COMM, KC_DOT,           KC_SLSH,          RSFT_T(KC_UP),
    KC_LCTL,          KC_LGUI, KC_LALT,                   KC_SPC,             MT_OSL_RALT, LT(FN1, KC_LEFT), LT(FN2, KC_DOWN), RCTL_T(KC_RGHT)
),
 [FN1] = LAYOUT_60_ansi( /* FN1 */
    KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,
    _______, _______, KC_UP,   _______, _______, _______, _______, _______, _______, _______, KC_PSCR, KC_HOME, KC_END,  _______,
    _______, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______, _______, _______, _______, _______, KC_PGUP, KC_PGDN, _______,
    _______,          KC_VOLU, KC_VOLD, KC_MUTE, _______, KC_MPLY, KC_MPRV, KC_MNXT, _______, KC_INS,  KC_DEL,  _______,
    _______, _______, _______,                            _______,                   _______, _______, MO(FN2), _______
),
 [FN2] = LAYOUT_60_ansi( /* FN2 */
    _______, KC_AP2_BT1, KC_AP2_BT2, KC_AP2_BT3, KC_AP2_BT4, _______, _______, _______, _______, KC_AP_RGB_MOD, KC_AP_RGB_TOG, KC_AP_RGB_VAD, KC_AP_RGB_VAI, _______,
    _______, _______,    KC_UP,      _______,    _______,    _______, _______, _______, _______, _______,       KC_PSCR,       KC_HOME,       KC_END,        _______,
    _______, KC_LEFT,    KC_DOWN,    KC_RGHT,    _______,    _______, _______, _______, _______, _______,       KC_PGUP,       KC_PGDN,       _______,
    _______,             _______,    _______,    _______,    _______, _______, _______, _______, _______,       KC_INS,        KC_DEL,        _______,
    _______, _______,    _______,                                     _______,                   _______,       _______,       _______,       _______
 ),
};
// clang-format on

// Helper function to set BASE layer constants (arrow keys, layer indicators)
void set_base_layer_leds(void) {
    // Arrow keys on modifiers - constant red
    ap2_led_sticky_set_key(3, 12, COLOR_RED); // Right Shift (Up)
    ap2_led_sticky_set_key(4, 12, COLOR_RED); // Right Ctrl (Right)
    ap2_led_sticky_set_key(4, 10, COLOR_RED); // FN1 (Left)
    ap2_led_sticky_set_key(4, 11, COLOR_RED); // FN2 (Down)
    
    // Layer indicator keys - red in BASE
    ap2_led_sticky_set_key(2, 0, COLOR_RED);  // Caps Lock
    // FN1 and FN2 already set above as arrow keys
    
    // Right Alt (OSL FN1) - orange to indicate one-shot layer capability
    ap2_led_sticky_set_key(4, 7, COLOR_ORANGE);
    
    // Number row - normal (not lit) in BASE, will light up green when FN1 is pressed
}

// Helper function to clear all sticky keys
void clear_all_sticky_keys(void) {
    ap2_led_unset_sticky_all();
}

// Helper function to set FN1 layer colors
void set_fn1_layer_leds(void) {
    // Number row (1-0, -, =) - green when FN1 is active
    for (uint8_t col = 1; col <= 12; col++) {
        ap2_led_mask_set_key(0, col, COLOR_GREEN);
    }
    // set esc in yello when fn1 is active and light blue whe fn1 and shift is active
    ap2_led_mask_set_key(0, 0, COLOR_YELLOW); // ESC
    
    // WASD arrow keys - red
    ap2_led_mask_set_key(1, 2, COLOR_RED);  // W (Up)
    ap2_led_mask_set_key(2, 1, COLOR_RED);  // A (Left)
    ap2_led_mask_set_key(2, 2, COLOR_RED);  // S (Down)
    ap2_led_mask_set_key(2, 3, COLOR_RED);  // D (Right)
    
    // Volume controls - yellow for Vol+ and Vol-, blue for Mute
    ap2_led_mask_set_key(3, 2, COLOR_YELLOW);  // Volume Up (Z position)
    ap2_led_mask_set_key(3, 3, COLOR_YELLOW);  // Volume Down (X position)
    ap2_led_mask_set_key(3, 4, COLOR_BLUE);    // Mute (C position) - blue to mark as mute
    
    // Media controls - green for Play/Pause, orange for Prev/Next
    ap2_led_mask_set_key(3, 6, COLOR_GREEN);   // Play/Pause (B position)
    ap2_led_mask_set_key(3, 7, COLOR_ORANGE);  // Previous track (N position)
    ap2_led_mask_set_key(3, 8, COLOR_ORANGE);  // Next track (M position)
    
    // Navigation/function keys in FN1 - purple
    // p, [, ], ;, ', /, . keys are PSCR, HOME, END, PGUP, PGDN, DEL, INS in FN1
    ap2_led_mask_set_key(1, 10, COLOR_PURPLE);  // p key (PSCR in FN1)
    ap2_led_mask_set_key(1, 11, COLOR_PURPLE);  // [ key (HOME in FN1)
    ap2_led_mask_set_key(1, 12, COLOR_PURPLE);  // ] key (END in FN1)
    ap2_led_mask_set_key(2, 10, COLOR_PURPLE);  // ; key (PGUP in FN1)
    ap2_led_mask_set_key(2, 11, COLOR_PURPLE);  // ' key (PGDN in FN1)
    ap2_led_mask_set_key(3, 10, COLOR_PURPLE);    // . key (INS in FN1)
    ap2_led_mask_set_key(3, 11, COLOR_PURPLE);  // / key (DEL in FN1)
    
    // Layer indicators - cyan for active FN1
    ap2_led_sticky_set_key(2, 0, COLOR_CYAN);  // Caps Lock
    ap2_led_sticky_set_key(4, 10, COLOR_CYAN); // FN1 key
    ap2_led_sticky_set_key(4, 7, COLOR_CYAN);  // Right Alt (OSL trigger)
    
    // Space bar - cyan
    ap2_led_mask_set_key(4, 6, COLOR_CYAN);
}

// Helper function to set FN2 layer colors
void set_fn2_layer_leds(void) {
    // RGB control keys (8, 9, 0, -, =) - purple to indicate LED change colors
    ap2_led_mask_set_key(0, 8, COLOR_PURPLE);   // 8 key (RGB MOD)
    ap2_led_mask_set_key(0, 9, COLOR_PURPLE);   // 9 key (RGB TOG)
    ap2_led_mask_set_key(0, 10, COLOR_PURPLE);  // 0 key (RGB VAD)
    ap2_led_mask_set_key(0, 11, COLOR_PURPLE);  // - key (RGB VAI)
    ap2_led_mask_set_key(0, 12, COLOR_PURPLE);  // = key
    
    // Layer indicator - purple for active FN2
    ap2_led_sticky_set_key(4, 11, COLOR_PURPLE); // FN2 key
    
    // Space bar - purple
    ap2_led_mask_set_key(4, 6, COLOR_PURPLE);
}

// Helper function to reset to BASE layer colors
void reset_to_base_colors(void) {
    // Clear all layer-specific colors first
    ap2_led_reset_foreground_color();
    
    // Re-set BASE layer constants
    set_base_layer_leds();
    
    // Reset space bar (will use profile color)
    ap2_led_unset_sticky_key(4, 6);
    
    // Reset Right Alt to orange (OSL indicator)
    ap2_led_sticky_set_key(4, 7, COLOR_ORANGE);
}

void keyboard_post_init_user(void) {
    ap2_led_enable();
    ap2_led_set_profile(7);
    
    // Set BASE layer constant LEDs
    set_base_layer_leds();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    // Clear previous layer colors
    ap2_led_reset_foreground_color();
    
    switch (get_highest_layer(state)) {
        case FN1:
            set_fn1_layer_leds();
            break;
        case FN2:
            set_fn2_layer_leds();
            break;
        default:
            reset_to_base_colors();
            break;
    }
    return state;
}

// Handle key press feedback and custom key combinations
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Manual one-shot layer: turn off FN1 after any non-modifier key is released
    if (osl_fn1_active && !record->event.pressed && keycode != MT_OSL_RALT) {
        // Check if it's not a modifier key
        if (!is_modifier_keycode(keycode)) {
            // Key was released, turn off the one-shot layer
            layer_off(FN1);
            osl_fn1_active = false;
        }
    }
    
    // Force media keys to work when FN1 layer is active
    // This ensures they work even if VIA or something else tries to override
    if (layer_state_is(FN1) && record->event.pressed) {
        switch (keycode) {
            case KC_B:  // B key on base layer, but should be KC_MPLY on FN1
                // Check if we're actually on FN1 layer
                if (get_highest_layer(layer_state) == FN1) {
                    tap_code(KC_MPLY);
                    return false;  // Prevent default handling
                }
                break;
            case KC_N:  // N key on base layer, but should be KC_MPRV on FN1
                if (get_highest_layer(layer_state) == FN1) {
                    tap_code(KC_MPRV);
                    return false;
                }
                break;
            case KC_M:  // M key on base layer, but should be KC_MNXT on FN1
                if (get_highest_layer(layer_state) == FN1) {
                    tap_code(KC_MNXT);
                    return false;
                }
                break;
        }
    }
    
    switch (keycode) {
        case KC_ESC:
            if (record->event.pressed) {
                uint8_t mods = get_mods();
                if (mods & MOD_MASK_CTRL) {
                    // Ctrl + Esc = ` (grave)
                    del_mods(MOD_MASK_CTRL);
                    tap_code(KC_GRV);
                    set_mods(mods); // Restore original mods
                    return false;
                } else if (mods & MOD_MASK_SHIFT) {
                    // Shift + Esc = ~ (tilde)
                    // Shift is already held, grave becomes tilde
                    tap_code(KC_GRV);
                    return false;
                }
            }
            return true;
        
        case MT_OSL_RALT:
            // Right Alt on hold, One-Shot FN1 layer on tap
            if (record->tap.count && record->event.pressed) {
                if (osl_fn1_active) {
                    // Already in OSL mode, turn it off
                    layer_off(FN1);
                    osl_fn1_active = false;
                } else {
                    // Activate manual one-shot FN1 layer
                    layer_on(FN1);
                    osl_fn1_active = true;
                }
                return false;
            }
            // Hold: let it act as normal Right Alt (handled by RALT_T)
            break;
        
        default:
            return true;
    }
    return true;
}

bool led_update_user(led_t leds) {
    // Handle caps lock LED if needed
    // The layer state handler will manage layer indicator colors
    return true;
}

