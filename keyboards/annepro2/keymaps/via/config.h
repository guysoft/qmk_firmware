#pragma once

// Tapping term - adjust if needed (default is usually 200ms)
#define TAPPING_TERM 200

// Hold on other key press - if you press another key while holding Right Alt,
// it immediately switches to modifier behavior
#define HOLD_ON_OTHER_KEY_PRESS

// Optional: Permissive hold can help with tap detection
// #define PERMISSIVE_HOLD

// MIDI Configuration (as per QMK documentation)
// MIDI_ADVANCED enables processing of MIDI keycodes from keymap
#define MIDI_ADVANCED

// Layer optimization: Use 8-bit layer state (supports up to 8 layers)
// We have 5 layers: BASE, FN1, FN2, PIANO, ARTSEY - so 8-bit is sufficient
#define LAYER_STATE_8BIT

// Combo timing - time window for keys to be pressed together (in milliseconds)
// Lower value = faster response, but less time to press keys simultaneously
#define COMBO_TERM 100  // Reduced to 30ms to minimize delay on base layer keys

// Note: Full debug output is too large for firmware size
// Use MIDI monitoring tools instead (aseqdump, etc.)

