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
#ifdef SEQUENCER_ENABLE
#    include "sequencer.h"
#endif
#ifdef MIDI_ENABLE
#    include "qmk_midi.h"
#endif
#ifdef COMBO_ENABLE
#    include "process_combo.h"
#endif

// Manual one-shot layer tracking (since built-in OSL doesn't work well on AP2)
static bool osl_fn1_active = false;

// Sequencer mode state
#ifdef SEQUENCER_ENABLE
static bool sequencer_mode_active = false;
static uint8_t last_sequencer_step = 255;  // Track last step for LED updates
static bool last_sequencer_state = false;  // Track sequencer on/off state
#endif

// Piano mode state
#ifdef MIDI_ENABLE
static bool piano_mode_active = false;
#endif

// Artsey mode state
static bool artsey_mode_active = false;

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
    PIANO,
    // ARTSEY layer removed - we handle Artsey mode on BASE layer instead
};

// Custom keycodes
enum custom_keycodes {
    OSL_RALT = SAFE_RANGE,  // Right Alt on hold, One-Shot FN1 on tap
    SEQ_ON,                 // Enter sequencer mode
    SEQ_OFF,                 // Exit sequencer mode
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
#define COLOR_WHITE  ((ap2_led_t){.p.red = 0xff, .p.green = 0xff, .p.blue = 0xff, .p.alpha = 0xff})
#define COLOR_DIM_RED ((ap2_led_t){.p.red = 0x40, .p.green = 0x00, .p.blue = 0x00, .p.alpha = 0xff})
#define COLOR_PINK ((ap2_led_t){.p.red = 0xff, .p.green = 0x69, .p.blue = 0xb4, .p.alpha = 0xff})

// Artsey key aliases - map physical keys to Artsey positions
// Top row: Q W E R -> BASE_1_1 BASE_1_2 BASE_1_3 BASE_1_4
// Bottom row: A S D F -> BASE_2_1 BASE_2_2 BASE_2_3 BASE_2_4
#define BASE_1_1 KC_Q  // Q key -> Artsey S
#define BASE_1_2 KC_W  // W key -> Artsey T
#define BASE_1_3 KC_E  // E key -> Artsey R
#define BASE_1_4 KC_R  // R key -> Artsey A
#define BASE_2_1 KC_A  // A key -> Artsey O
#define BASE_2_2 KC_S  // S key -> Artsey I
#define BASE_2_3 KC_D  // D key -> Artsey Y
#define BASE_2_4 KC_F  // F key -> Artsey E

#ifdef SEQUENCER_ENABLE
// Step-to-key mapping: 16 steps mapped to physical keys
// More even layout: 8 steps per row
// Steps 0-7: Top row letters (Q-P keys) - row 1, columns 1-8
// Steps 8-15: Second row letters (A-L keys) - row 2, columns 1-8
typedef struct {
    uint8_t row;
    uint8_t col;
} step_key_map_t;

static const step_key_map_t step_key_map[16] = {
    {1, 1},  // Step 0 -> Q key
    {1, 2},  // Step 1 -> W key
    {1, 3},  // Step 2 -> E key
    {1, 4},  // Step 3 -> R key
    {1, 5},  // Step 4 -> T key
    {1, 6},  // Step 5 -> Y key
    {1, 7},  // Step 6 -> U key
    {1, 8},  // Step 7 -> I key
    {2, 1},  // Step 8 -> A key
    {2, 2},  // Step 9 -> S key
    {2, 3},  // Step 10 -> D key
    {2, 4},  // Step 11 -> F key
    {2, 5},  // Step 12 -> G key
    {2, 6},  // Step 13 -> H key
    {2, 7},  // Step 14 -> J key
    {2, 8},  // Step 15 -> K key
};
#endif

// Piano mode functions
#ifdef MIDI_ENABLE
// Forward declarations for piano functions
void piano_mode_on(void);
void piano_mode_off(void);
void set_piano_layer_leds(void);
void reset_to_base_colors(void);  // Forward declare
void clear_all_sticky_keys(void);  // Forward declare

// Enter piano mode
void piano_mode_on(void) {
    piano_mode_active = true;
    layer_on(PIANO);
    // Clear all previous LED states (including sticky) to avoid leftover colors
    clear_all_sticky_keys();
    ap2_led_reset_foreground_color();
    set_piano_layer_leds();
}

// Exit piano mode
void piano_mode_off(void) {
    piano_mode_active = false;
    layer_off(PIANO);
    reset_to_base_colors();
}

// Set LED colors for piano layer
void set_piano_layer_leds(void) {
    // White keys (C, D, E, F, G, A, B) - White LED
    ap2_led_mask_set_key(2, 1, COLOR_WHITE);  // A - C4
    ap2_led_mask_set_key(2, 2, COLOR_WHITE);  // S - D4
    ap2_led_mask_set_key(2, 3, COLOR_WHITE);  // D - E4
    ap2_led_mask_set_key(2, 4, COLOR_WHITE);  // F - F4
    ap2_led_mask_set_key(2, 5, COLOR_WHITE);  // G - G4
    ap2_led_mask_set_key(2, 6, COLOR_WHITE);  // H - A4
    ap2_led_mask_set_key(2, 7, COLOR_WHITE);  // J - B4
    ap2_led_mask_set_key(2, 8, COLOR_WHITE);  // K - C5
    ap2_led_mask_set_key(2, 9, COLOR_WHITE);  // L - D5
    ap2_led_mask_set_key(2, 10, COLOR_WHITE); // ; - E5
    ap2_led_mask_set_key(2, 11, COLOR_WHITE); // ' - F5
    
    // Sharp keys (C#, D#, F#, G#, A#) - Yellow LED
    ap2_led_mask_set_key(1, 2, COLOR_YELLOW);  // W - C#4
    ap2_led_mask_set_key(1, 3, COLOR_YELLOW);  // E - D#4
    ap2_led_mask_set_key(1, 5, COLOR_YELLOW);  // T - F#4 (mapped to T key before G)
    ap2_led_mask_set_key(1, 6, COLOR_YELLOW);  // Y - G#4
    ap2_led_mask_set_key(1, 7, COLOR_YELLOW);  // U - A#4
    ap2_led_mask_set_key(1, 9, COLOR_YELLOW);  // O - C#5
    ap2_led_mask_set_key(1, 10, COLOR_YELLOW); // P - D#5
    ap2_led_mask_set_key(1, 12, COLOR_YELLOW); // ] - F#5
    
    // Control keys - Cyan
    ap2_led_mask_set_key(3, 1, COLOR_CYAN);  // Z - Octave down
    ap2_led_mask_set_key(3, 2, COLOR_CYAN);  // X - Octave up
    ap2_led_mask_set_key(3, 7, COLOR_CYAN);  // M - Exit (with FN2)
    
    // Explicitly clear N key (column 6) to prevent FN1 layer LED from showing
    ap2_led_unset_sticky_key(3, 6);  // N key - clear any sticky LED from other layers
}
#endif

// Artsey mode functions
// Forward declarations
void artsey_mode_on(void);
void artsey_mode_off(void);
void set_artsey_layer_leds(void);
void reset_to_base_colors(void);  // Forward declare

// Enter Artsey mode
void artsey_mode_on(void) {
    artsey_mode_active = true;
    // Don't activate a layer - we handle Artsey on BASE layer
    clear_all_sticky_keys();
    ap2_led_reset_foreground_color();
    set_artsey_layer_leds();
}

// Exit Artsey mode
void artsey_mode_off(void) {
    artsey_mode_active = false;
    // No layer to deactivate
    reset_to_base_colors();
}

// Set LED colors for Artsey layer
void set_artsey_layer_leds(void) {
    // QWER/ASDF keys - Orange LED (matching FN1 style)
    ap2_led_mask_set_key(1, 1, COLOR_ORANGE);  // Q - Artsey S
    ap2_led_mask_set_key(1, 2, COLOR_ORANGE);  // W - Artsey T
    ap2_led_mask_set_key(1, 3, COLOR_ORANGE);  // E - Artsey R
    ap2_led_mask_set_key(1, 4, COLOR_ORANGE);  // R - Artsey A
    ap2_led_mask_set_key(2, 1, COLOR_ORANGE);  // A - Artsey O
    ap2_led_mask_set_key(2, 2, COLOR_ORANGE);  // S - Artsey I
    ap2_led_mask_set_key(2, 3, COLOR_ORANGE);  // D - Artsey Y
    ap2_led_mask_set_key(2, 4, COLOR_ORANGE);  // F - Artsey E
}

#ifdef COMBO_ENABLE
// Artsey combo system
enum combo_events {
    ARTSEY_H,
    ARTSEY_Q,
    ARTSEY_U,
    ARTSEY_C,
    ARTSEY_K,  // K = D+A (O+Y in Artsey)
    ARTSEY_B,
    ARTSEY_W,
    ARTSEY_N,
    ARTSEY_F,
    ARTSEY_X,  // X = Q+W+E (S+T+R in Artsey)
    ARTSEY_J,
    ARTSEY_M,  // M = A+S+D (O+I+Y in Artsey)
    ARTSEY_P,  // P = A+S+F (O+I+E in Artsey)
    ARTSEY_V,  // V = Q+E (S+R in Artsey)
    ARTSEY_L,  // L = F+D+S (E+Y+I in Artsey)
    ARTSEY_Z,  // Z = Q+W+E+R (S+T+R+A in Artsey)
    ARTSEY_D,  // D = R+E+W (A+R+T in Artsey)
    ARTSEY_G,
    ARTSEY_SPACE,
    ARTSEY_BACKSPACE,
    ARTSEY_ENTER,
    // Punctuation
    ARTSEY_QUOTE,   // ' - E+D (Y+R)
    ARTSEY_BANG,    // ! - W+S (I+T)
    ARTSEY_QUEST,   // ? - Q+A (O+S)
    ARTSEY_PERIOD,  // . - R+S (I+A)
    ARTSEY_COMMA,   // , - R+D (Y+A)
    ARTSEY_SLASH,   // / - R+A (O+A)
    // Control keys
    ARTSEY_TAB,     // Tab - R+E+W+A
    ARTSEY_ESCAPE,  // Escape - R+E+A
    ARTSEY_DEL,     // Delete - E+F
    // Modifiers
    ARTSEY_OS_SHIFT, // One-shot shift - F+E+W+Q
    ARTSEY_CTRL,     // Ctrl toggle - Q+F
    ARTSEY_GUI,      // GUI/Win toggle - Q+D
    ARTSEY_ALT,      // Alt toggle - Q+S
    ARTSEY_SHIFT,    // Shift toggle - R+D+S+A
    ARTSEY_PANIC,    // Clear all modifiers - All 8 keys
    COMBO_LENGTH
};
uint16_t COMBO_LEN = COMBO_LENGTH;

// Combo definitions - using BASE_X_Y aliases
const uint16_t PROGMEM artsey_h[] = {BASE_2_4, BASE_2_2, COMBO_END};
const uint16_t PROGMEM artsey_q[] = {BASE_1_4, BASE_1_2, BASE_1_1, COMBO_END};
const uint16_t PROGMEM artsey_u[] = {BASE_2_3, BASE_2_2, COMBO_END};
const uint16_t PROGMEM artsey_c[] = {BASE_2_4, BASE_2_3, COMBO_END};
const uint16_t PROGMEM artsey_k[] = {BASE_2_3, BASE_2_1, COMBO_END};  // K = D+A (O+Y in Artsey)
const uint16_t PROGMEM artsey_b[] = {BASE_2_4, BASE_2_1, COMBO_END};
const uint16_t PROGMEM artsey_w[] = {BASE_1_4, BASE_1_1, COMBO_END};
const uint16_t PROGMEM artsey_n[] = {BASE_2_2, BASE_2_1, COMBO_END};
const uint16_t PROGMEM artsey_f[] = {BASE_1_4, BASE_1_3, COMBO_END};
const uint16_t PROGMEM artsey_x[] = {BASE_1_3, BASE_1_2, BASE_1_1, COMBO_END};  // X = Q+W+E (S+T+R in Artsey)
const uint16_t PROGMEM artsey_j[] = {BASE_1_2, BASE_1_1, COMBO_END};
const uint16_t PROGMEM artsey_m[] = {BASE_2_3, BASE_2_2, BASE_2_1, COMBO_END};  // M = A+S+D (O+I+Y in Artsey)
const uint16_t PROGMEM artsey_p[] = {BASE_2_4, BASE_2_2, BASE_2_1, COMBO_END};  // P = A+S+F (O+I+E in Artsey)
const uint16_t PROGMEM artsey_v[] = {BASE_1_3, BASE_1_1, COMBO_END};  // V = Q+E (S+R in Artsey)
const uint16_t PROGMEM artsey_l[] = {BASE_2_4, BASE_2_3, BASE_2_2, COMBO_END};  // L = F+D+S (E+Y+I in Artsey)
const uint16_t PROGMEM artsey_z[] = {BASE_1_4, BASE_1_3, BASE_1_2, BASE_1_1, COMBO_END};  // Z = Q+W+E+R (S+T+R+A in Artsey)
const uint16_t PROGMEM artsey_d[] = {BASE_1_4, BASE_1_3, BASE_1_2, COMBO_END};  // D = R+E+W (A+R+T in Artsey)
const uint16_t PROGMEM artsey_g[] = {BASE_1_3, BASE_1_2, COMBO_END};
const uint16_t PROGMEM artsey_space[] = {BASE_2_4, BASE_2_3, BASE_2_2, BASE_2_1, COMBO_END};
const uint16_t PROGMEM artsey_backspace[] = {BASE_2_4, BASE_1_3, COMBO_END};  // Fixed: F+E (E+R in Artsey)
const uint16_t PROGMEM artsey_enter[] = {BASE_1_4, BASE_2_4, COMBO_END};
// Punctuation
const uint16_t PROGMEM artsey_quote[] = {BASE_1_3, BASE_2_3, COMBO_END};   // E+D (Y+R in Artsey)
const uint16_t PROGMEM artsey_bang[] = {BASE_1_2, BASE_2_2, COMBO_END};     // W+S (I+T in Artsey)
const uint16_t PROGMEM artsey_quest[] = {BASE_1_1, BASE_2_1, COMBO_END};   // Q+A (O+S in Artsey)
const uint16_t PROGMEM artsey_period[] = {BASE_1_4, BASE_2_2, COMBO_END};   // R+S (I+A in Artsey)
const uint16_t PROGMEM artsey_comma[] = {BASE_1_4, BASE_2_3, COMBO_END};   // R+D (Y+A in Artsey)
const uint16_t PROGMEM artsey_slash[] = {BASE_1_4, BASE_2_1, COMBO_END};   // R+A (O+A in Artsey)
// Control keys
const uint16_t PROGMEM artsey_tab[] = {BASE_1_4, BASE_1_3, BASE_1_2, BASE_2_1, COMBO_END};  // R+E+W+A
const uint16_t PROGMEM artsey_escape[] = {BASE_1_4, BASE_1_3, BASE_2_1, COMBO_END};         // R+E+A
const uint16_t PROGMEM artsey_del[] = {BASE_2_2, BASE_1_3, COMBO_END};                    // S+E (I+R in Artsey)
// Modifiers
const uint16_t PROGMEM artsey_os_shift[] = {BASE_2_4, BASE_1_3, BASE_1_2, BASE_1_1, COMBO_END}; // F+E+W+Q
const uint16_t PROGMEM artsey_ctrl[] = {BASE_1_1, BASE_2_4, COMBO_END};                         // Q+F
const uint16_t PROGMEM artsey_gui[] = {BASE_1_1, BASE_2_3, COMBO_END};                          // Q+D
const uint16_t PROGMEM artsey_alt[] = {BASE_1_1, BASE_2_2, COMBO_END};                          // Q+S
const uint16_t PROGMEM artsey_shift[] = {BASE_1_4, BASE_2_3, BASE_2_2, BASE_2_1, COMBO_END};    // R+D+S+A
const uint16_t PROGMEM artsey_panic[] = {BASE_1_4, BASE_1_3, BASE_1_2, BASE_1_1, BASE_2_4, BASE_2_3, BASE_2_2, BASE_2_1, COMBO_END}; // All 8 keys

combo_t key_combos[] = {
    [ARTSEY_H] = COMBO_ACTION(artsey_h),
    [ARTSEY_Q] = COMBO_ACTION(artsey_q),
    [ARTSEY_U] = COMBO_ACTION(artsey_u),
    [ARTSEY_C] = COMBO_ACTION(artsey_c),
    [ARTSEY_K] = COMBO_ACTION(artsey_k),
    [ARTSEY_B] = COMBO_ACTION(artsey_b),
    [ARTSEY_W] = COMBO_ACTION(artsey_w),
    [ARTSEY_N] = COMBO_ACTION(artsey_n),
    [ARTSEY_F] = COMBO_ACTION(artsey_f),
    [ARTSEY_X] = COMBO_ACTION(artsey_x),
    [ARTSEY_J] = COMBO_ACTION(artsey_j),
    [ARTSEY_M] = COMBO_ACTION(artsey_m),
    [ARTSEY_P] = COMBO_ACTION(artsey_p),
    [ARTSEY_V] = COMBO_ACTION(artsey_v),
    [ARTSEY_L] = COMBO_ACTION(artsey_l),
    [ARTSEY_Z] = COMBO_ACTION(artsey_z),
    [ARTSEY_D] = COMBO_ACTION(artsey_d),
    [ARTSEY_G] = COMBO_ACTION(artsey_g),
    [ARTSEY_SPACE] = COMBO_ACTION(artsey_space),
    [ARTSEY_BACKSPACE] = COMBO_ACTION(artsey_backspace),
    [ARTSEY_ENTER] = COMBO_ACTION(artsey_enter),
    // Punctuation
    [ARTSEY_QUOTE] = COMBO_ACTION(artsey_quote),
    [ARTSEY_BANG] = COMBO_ACTION(artsey_bang),
    [ARTSEY_QUEST] = COMBO_ACTION(artsey_quest),
    [ARTSEY_PERIOD] = COMBO_ACTION(artsey_period),
    [ARTSEY_COMMA] = COMBO_ACTION(artsey_comma),
    [ARTSEY_SLASH] = COMBO_ACTION(artsey_slash),
    // Control keys
    [ARTSEY_TAB] = COMBO_ACTION(artsey_tab),
    [ARTSEY_ESCAPE] = COMBO_ACTION(artsey_escape),
    [ARTSEY_DEL] = COMBO_ACTION(artsey_del),
    // Modifiers
    [ARTSEY_OS_SHIFT] = COMBO_ACTION(artsey_os_shift),
    [ARTSEY_CTRL] = COMBO_ACTION(artsey_ctrl),
    [ARTSEY_GUI] = COMBO_ACTION(artsey_gui),
    [ARTSEY_ALT] = COMBO_ACTION(artsey_alt),
    [ARTSEY_SHIFT] = COMBO_ACTION(artsey_shift),
    [ARTSEY_PANIC] = COMBO_ACTION(artsey_panic),
};

void process_combo_event(uint16_t combo_index, bool pressed) {
    // Only process Artsey combos when Artsey mode is active
    // But first check if combos are being detected at all
    if (!artsey_mode_active) {
        return;
    }
    
    switch(combo_index) {
        case ARTSEY_H:
            if (pressed) { 
                SEND_STRING("h"); 
            }
            break;
        case ARTSEY_Q:
            if (pressed) { 
                SEND_STRING("q"); 
            }
            break;
        case ARTSEY_U:
            if (pressed) { 
                SEND_STRING("u"); 
            }
            break;
        case ARTSEY_C:
            if (pressed) { 
                SEND_STRING("c"); 
            }
            break;
        case ARTSEY_K:
            if (pressed) { 
                SEND_STRING("k"); 
            }
            break;
        case ARTSEY_B:
            if (pressed) { 
                SEND_STRING("b"); 
            }
            break;
        case ARTSEY_W:
            if (pressed) { 
                SEND_STRING("w"); 
            }
            break;
        case ARTSEY_N:
            if (pressed) { 
                SEND_STRING("n"); 
            }
            break;
        case ARTSEY_F:
            if (pressed) { 
                SEND_STRING("f"); 
            }
            break;
        case ARTSEY_X:
            if (pressed) { 
                SEND_STRING("x"); 
            }
            break;
        case ARTSEY_J:
            if (pressed) { 
                SEND_STRING("j"); 
            }
            break;
        case ARTSEY_M:
            if (pressed) { 
                SEND_STRING("m"); 
            }
            break;
        case ARTSEY_P:
            if (pressed) { 
                SEND_STRING("p"); 
            }
            break;
        case ARTSEY_V:
            if (pressed) { 
                SEND_STRING("v"); 
            }
            break;
        case ARTSEY_L:
            if (pressed) { 
                SEND_STRING("l"); 
            }
            break;
        case ARTSEY_Z:
            if (pressed) { 
                SEND_STRING("z"); 
            }
            break;
        case ARTSEY_D:
            if (pressed) { 
                SEND_STRING("d"); 
            }
            break;
        case ARTSEY_G:
            if (pressed) { 
                SEND_STRING("g"); 
            }
            break;
        case ARTSEY_SPACE:
            if (pressed) { 
                SEND_STRING(" ");  // Use SEND_STRING like reference firmware
            }
            break;
        case ARTSEY_BACKSPACE:
            if (pressed) { 
                tap_code(KC_BSPC);  // Use tap_code for backspace
            }
            break;
        case ARTSEY_ENTER:
            if (pressed) { 
                tap_code(KC_ENT);  // Use tap_code for enter
            }
            break;
        // Punctuation
        case ARTSEY_QUOTE:
            if (pressed) { 
                SEND_STRING("'"); 
            }
            break;
        case ARTSEY_BANG:
            if (pressed) { 
                SEND_STRING("!"); 
            }
            break;
        case ARTSEY_QUEST:
            if (pressed) { 
                SEND_STRING("?"); 
            }
            break;
        case ARTSEY_PERIOD:
            if (pressed) { 
                SEND_STRING("."); 
            }
            break;
        case ARTSEY_COMMA:
            if (pressed) { 
                SEND_STRING(","); 
            }
            break;
        case ARTSEY_SLASH:
            if (pressed) { 
                SEND_STRING("/"); 
            }
            break;
        // Control keys
        case ARTSEY_TAB:
            if (pressed) { 
                tap_code(KC_TAB); 
            }
            break;
        case ARTSEY_ESCAPE:
            if (pressed) { 
                tap_code(KC_ESC); 
            }
            break;
        case ARTSEY_DEL:
            if (pressed) { 
                tap_code(KC_DEL); 
            }
            break;
        // Modifiers
        case ARTSEY_OS_SHIFT:
            if (pressed) { 
                add_oneshot_mods(MOD_BIT(KC_LSFT)); 
            }
            break;
        case ARTSEY_CTRL:
            if (pressed) {
                if (get_mods() & MOD_MASK_CTRL) {
                    del_mods(MOD_MASK_CTRL);
                } else {
                    add_mods(MOD_MASK_CTRL);
                }
            }
            break;
        case ARTSEY_GUI:
            if (pressed) {
                if (get_mods() & MOD_MASK_GUI) {
                    del_mods(MOD_MASK_GUI);
                } else {
                    add_mods(MOD_MASK_GUI);
                }
            }
            break;
        case ARTSEY_ALT:
            if (pressed) {
                if (get_mods() & MOD_MASK_ALT) {
                    del_mods(MOD_MASK_ALT);
                } else {
                    add_mods(MOD_MASK_ALT);
                }
            }
            break;
        case ARTSEY_SHIFT:
            if (pressed) {
                if (get_mods() & MOD_MASK_SHIFT) {
                    del_mods(MOD_MASK_SHIFT);
                } else {
                    add_mods(MOD_MASK_SHIFT);
                }
            }
            break;
        case ARTSEY_PANIC:
            if (pressed) { 
                clear_mods(); 
            }
            break;
    }
}
#endif

#ifdef SEQUENCER_ENABLE
// Forward declarations (needed because functions call each other)
void update_sequencer_leds(void);
void sequencer_mode_on(void);
void sequencer_mode_off(void);
void reset_to_base_colors(void);  // Declared later, forward declare here

// Enter sequencer mode
void sequencer_mode_on(void) {
    sequencer_mode_active = true;
    // Activate track 0 by default for programming
    sequencer_activate_track(0);
    // Clear previous LED state and show sequencer visualization
    ap2_led_reset_foreground_color();
    // Set a simple indicator: make all step keys dim red initially
    for (uint8_t step = 0; step < 16; step++) {
        step_key_map_t key_pos = step_key_map[step];
        ap2_led_mask_set_key(key_pos.row, key_pos.col, COLOR_DIM_RED);
    }
    // Don't start the sequencer automatically - let user press space to start
    // sequencer_on();  // Commented out - user starts with space bar
    // Update LEDs to show current state
    update_sequencer_leds();
}

// Exit sequencer mode
void sequencer_mode_off(void) {
    sequencer_mode_active = false;
    sequencer_off();  // Stop the sequencer
    // Restore normal LED state
    reset_to_base_colors();
}

// Update LEDs to show sequencer state
void update_sequencer_leds(void) {
    if (!sequencer_mode_active) {
        return;
    }
    
    bool sequencer_running = is_sequencer_on();
    uint8_t current_step = sequencer_get_current_step();
    
    // Always update LEDs for smooth visualization (matrix_scan_user is called frequently)
    // Update all step LEDs
    for (uint8_t step = 0; step < 16; step++) {
        step_key_map_t key_pos = step_key_map[step];
        ap2_led_t step_color;
        
        // Check if step is enabled for any active track
        bool step_enabled = false;
        for (uint8_t track = 0; track < 8; track++) {
            if (is_sequencer_track_active(track) && is_sequencer_step_on_for_track(step, track)) {
                step_enabled = true;
                break;
            }
        }
        
        // Determine color based on state
        if (step == current_step && sequencer_running) {
            // Current playing step: bright white/cyan for visibility
            step_color = COLOR_WHITE;
        } else if (step_enabled) {
            // Enabled step: green
            step_color = COLOR_GREEN;
        } else {
            // Disabled step: dim red
            step_color = COLOR_DIM_RED;
        }
        
        ap2_led_mask_set_key(key_pos.row, key_pos.col, step_color);
    }
    
    // Update sequencer status indicator on space bar - use sticky for visibility
    if (sequencer_running) {
        ap2_led_sticky_set_key(4, 6, COLOR_CYAN);  // Cyan = sequencer running
    } else {
        ap2_led_sticky_set_key(4, 6, COLOR_PURPLE);  // Purple = sequencer stopped
    }
    
    // Show active track(s) on number row (keys 1-8) - use sticky to ensure visibility
    for (uint8_t track = 0; track < 8; track++) {
        if (is_sequencer_track_active(track)) {
            // Track 0-7 map to keys 1-8 (columns 1-8)
            ap2_led_sticky_set_key(0, track + 1, COLOR_YELLOW);  // Yellow = active track
        } else {
            // Don't set inactive tracks - let them be dim or default
            ap2_led_unset_sticky_key(0, track + 1);
        }
    }
    
    last_sequencer_state = sequencer_running;
    last_sequencer_step = current_step;
}
#endif

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
    _______,          KC_VOLD, KC_VOLU, KC_MUTE, _______, KC_MPLY, KC_MPRV, KC_MNXT, _______, KC_INS,  KC_DEL,  _______,
    _______, _______, _______,                            _______,                   _______, _______, MO(FN2), _______
),
 [FN2] = LAYOUT_60_ansi( /* FN2 */
    _______, KC_AP2_BT1, KC_AP2_BT2, KC_AP2_BT3, KC_AP2_BT4, _______, _______, _______, _______, KC_AP_RGB_MOD, KC_AP_RGB_TOG, KC_AP_RGB_VAD, KC_AP_RGB_VAI, _______,
    _______, _______,    KC_UP,      KC_E,       _______,    _______, _______, _______, _______, _______,       KC_PSCR,       KC_HOME,       KC_END,        _______,
    _______, KC_LEFT,    KC_DOWN,    KC_RGHT,    _______,    _______, _______, _______, _______, KC_K,          KC_PGUP,       KC_PGDN,       _______,
    _______,             _______,    _______,    _______,    _______, _______, _______, _______, _______,       KC_INS,        KC_DEL,        _______,
    _______, _______,    _______,                                     _______,                   _______,       _______,       _______,       _______
 ),
 [PIANO] = LAYOUT_60_ansi( /* PIANO - MIDI Piano Keyboard */
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, QK_MIDI_NOTE_C_SHARP_4, QK_MIDI_NOTE_D_SHARP_4, _______, QK_MIDI_NOTE_F_SHARP_4, QK_MIDI_NOTE_G_SHARP_4, QK_MIDI_NOTE_A_SHARP_4, _______, QK_MIDI_NOTE_C_SHARP_5, QK_MIDI_NOTE_D_SHARP_5, _______, QK_MIDI_NOTE_F_SHARP_5, _______,
    _______, QK_MIDI_NOTE_C_4, QK_MIDI_NOTE_D_4, QK_MIDI_NOTE_E_4, QK_MIDI_NOTE_F_4, QK_MIDI_NOTE_G_4, QK_MIDI_NOTE_A_4, QK_MIDI_NOTE_B_4, QK_MIDI_NOTE_C_5, QK_MIDI_NOTE_D_5, QK_MIDI_NOTE_E_5, QK_MIDI_NOTE_F_5, _______,
    _______, QK_MIDI_OCTAVE_DOWN, QK_MIDI_OCTAVE_UP, _______, _______, _______, _______, KC_M, _______, _______, _______, _______,
    _______, _______, _______,                                     _______,                   _______,       _______,       _______,       _______
 ),
 // ARTSEY layer removed - Artsey mode is handled on BASE layer via process_record_user
};
// clang-format on

// Forward declarations for LED functions
void set_base_layer_leds(void);
void set_fn1_layer_leds(void);
void set_fn2_layer_leds(void);
void set_artsey_layer_leds(void);  // Forward declare

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
    
#ifdef MIDI_ENABLE
    // Mark piano mode key with pink so it's visible
    ap2_led_mask_set_key(1, 8, COLOR_PINK);  // I key (row 1, col 8) - Piano mode
#endif
    
    // Mark Artsey mode key with orange so it's visible
    ap2_led_mask_set_key(1, 7, COLOR_ORANGE);  // U key (row 1, col 7) - Artsey mode
    
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
    
#ifdef MIDI_ENABLE
    // Initialize MIDI device (required for MIDI output)
    setup_midi();
#endif
}

layer_state_t layer_state_set_user(layer_state_t state) {
#ifdef SEQUENCER_ENABLE
    // Don't override LEDs if sequencer mode is active
    if (sequencer_mode_active) {
        return state;
    }
#endif

#ifdef MIDI_ENABLE
    // Don't override LEDs if piano mode is active
    if (piano_mode_active) {
        return state;
    }
#endif

    // Don't override LEDs if Artsey mode is active
    if (artsey_mode_active) {
        return state;
    }
    
    // Clear previous layer colors
    ap2_led_reset_foreground_color();
    
    switch (get_highest_layer(state)) {
        case FN1:
            set_fn1_layer_leds();
            break;
        case FN2:
            set_fn2_layer_leds();
            break;
#ifdef MIDI_ENABLE
        case PIANO:
            set_piano_layer_leds();
            break;
#endif
        default:
            // Check if Artsey mode is active (handled on BASE layer, not a separate layer)
            if (artsey_mode_active) {
                set_artsey_layer_leds();
            } else {
                reset_to_base_colors();
            }
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
    
    // FN2 + Backspace = Shift+F12
    if (layer_state_is(FN2) && record->event.pressed && keycode == KC_BSPC) {
        if (get_highest_layer(layer_state) == FN2) {
            register_code(KC_LSFT);
            tap_code(KC_F12);
            unregister_code(KC_LSFT);
            return false;  // Prevent default backspace
        }
    }
    
#ifdef MIDI_ENABLE
    // FN2 + I = Enter piano mode
    if (layer_state_is(FN2) && record->event.pressed && keycode == KC_I) {
        // Double-check layer state
        if (get_highest_layer(layer_state) == FN2) {
            // Activate piano mode
            if (!piano_mode_active) {
                piano_mode_on();
            }
            return false;  // Prevent default I key
        }
    }
    
    // FN2 + M = Exit piano mode
    // Check if FN2 is held (not necessarily highest layer, since PIANO might be active)
    if (piano_mode_active && layer_state_is(FN2) && record->event.pressed && keycode == KC_M) {
        piano_mode_off();
        return false;  // Prevent default M key
    }
    
    // ESC = Exit piano mode (alternative exit method)
    if (piano_mode_active && record->event.pressed && keycode == KC_ESC) {
        piano_mode_off();
        return false;  // Prevent default ESC key
    }
#endif

    // Artsey mode handlers
    // FN2 + U = Enter Artsey mode
    if (layer_state_is(FN2) && record->event.pressed && keycode == KC_U) {
        if (get_highest_layer(layer_state) == FN2) {
            if (!artsey_mode_active) {
                artsey_mode_on();
            }
            return false;  // Prevent default U key
        }
    }
    
    // FN2 + M = Exit Artsey mode
    if (artsey_mode_active && layer_state_is(FN2) && record->event.pressed && keycode == KC_M) {
        artsey_mode_off();
        return false;  // Prevent default M key
    }
    
    // In Artsey mode, intercept Q/W/E/R/A/S/D/F keys and output Artsey letters
    // This allows combos to work (they check BASE layer keycodes) while outputting Artsey letters
    if (artsey_mode_active && record->event.pressed) {
        switch(keycode) {
            case KC_Q:  // Q -> Artsey S
                SEND_STRING("s");
                return false;
            case KC_W:  // W -> Artsey T
                SEND_STRING("t");
                return false;
            case KC_E:  // E -> Artsey R
                SEND_STRING("r");
                return false;
            case KC_R:  // R -> Artsey A
                SEND_STRING("a");
                return false;
            case KC_A:  // A -> Artsey O
                SEND_STRING("o");
                return false;
            case KC_S:  // S -> Artsey I
                SEND_STRING("i");
                return false;
            case KC_D:  // D -> Artsey Y
                SEND_STRING("y");
                return false;
            case KC_F:  // F -> Artsey E
                SEND_STRING("e");
                return false;
        }
    }
    // Note: ESC is NOT intercepted for Artsey mode - it remains functional
    
#ifdef SEQUENCER_ENABLE
    // FN2 + E = Enter sequencer mode (intercept E key when FN2 is active)
    if (record->event.pressed && keycode == KC_E) {
        if (get_highest_layer(layer_state) == FN2) {
            // Simple test: turn all LEDs purple to confirm detection
            ap2_led_reset_foreground_color();
            for (uint8_t row = 0; row < 5; row++) {
                for (uint8_t col = 0; col < 14; col++) {
                    ap2_led_mask_set_key(row, col, COLOR_PURPLE);
                }
            }
            if (!sequencer_mode_active) {
                sequencer_mode_on();
            }
            return false;  // Prevent default E key
        }
    }
    
    // FN2 + K = Exit sequencer mode (intercept K key when FN2 is active)
    if (record->event.pressed && keycode == KC_K) {
        if (get_highest_layer(layer_state) == FN2) {
            if (sequencer_mode_active) {
                sequencer_mode_off();
            } else {
                // Test: restore base colors
                reset_to_base_colors();
            }
            return false;  // Prevent default K key
        }
    }
    
    
    // In sequencer mode, handle controls and step programming
    if (sequencer_mode_active && record->event.pressed) {
        // Track selection: Number row 1-8 selects tracks 0-7
        switch (keycode) {
            case KC_1: sequencer_toggle_single_active_track(0); return false;
            case KC_2: sequencer_toggle_single_active_track(1); return false;
            case KC_3: sequencer_toggle_single_active_track(2); return false;
            case KC_4: sequencer_toggle_single_active_track(3); return false;
            case KC_5: sequencer_toggle_single_active_track(4); return false;
            case KC_6: sequencer_toggle_single_active_track(5); return false;
            case KC_7: sequencer_toggle_single_active_track(6); return false;
            case KC_8: sequencer_toggle_single_active_track(7); return false;
            
            // Controls
            case KC_SPC: sequencer_toggle(); return false;  // Space = Play/Pause
            case KC_0: sequencer_set_all_steps_off(); return false;  // 0 = Clear all steps
            case KC_9: sequencer_set_all_steps_on(); return false;  // 9 = Enable all steps
            case KC_MINS: sequencer_decrease_tempo(); return false;  // - = Slower tempo
            case KC_EQL: sequencer_increase_tempo(); return false;   // = = Faster tempo
            // Test MIDI: L key sends a test C4 note (for debugging)
            case KC_L: 
                #if defined(MIDI_ENABLE) && defined(MIDI_BASIC)
                process_midi_basic_noteon(60);  // C4 = MIDI note 60
                midi_device_process(&midi_device);  // Force immediate processing
                #endif
                return false;
            
            // Step programming: Q-I and A-K toggle steps
            case KC_Q: sequencer_toggle_step(0); return false;
            case KC_W: sequencer_toggle_step(1); return false;
            case KC_E: sequencer_toggle_step(2); return false;
            case KC_R: sequencer_toggle_step(3); return false;
            case KC_T: sequencer_toggle_step(4); return false;
            case KC_Y: sequencer_toggle_step(5); return false;
            case KC_U: sequencer_toggle_step(6); return false;
            case KC_I: sequencer_toggle_step(7); return false;
            case KC_A: sequencer_toggle_step(8); return false;
            case KC_S: sequencer_toggle_step(9); return false;
            case KC_D: sequencer_toggle_step(10); return false;
            case KC_F: sequencer_toggle_step(11); return false;
            case KC_G: sequencer_toggle_step(12); return false;
            case KC_H: sequencer_toggle_step(13); return false;
            case KC_J: sequencer_toggle_step(14); return false;
            case KC_K: sequencer_toggle_step(15); return false;
        }
    }
#endif
    
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

#ifdef SEQUENCER_ENABLE
// Real-time LED updates for sequencer visualization
void matrix_scan_user(void) {
    if (sequencer_mode_active) {
        update_sequencer_leds();
    }
}
#endif

