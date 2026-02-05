# VIA_ENABLE = yes  # Disabled to save firmware space

# Enable Link Time Optimization for smaller firmware size
LTO_ENABLE = yes

# Disable unused features to save space
# CONSOLE_ENABLE = yes  # Disabled - adds ~5KB, causes firmware overflow
COMMAND_ENABLE = yes  # Enable QMK commands (qmk console, etc.) for debugging
MOUSEKEY_ENABLE = no
SPACE_CADET_ENABLE = no
GRAVE_ESC_ENABLE = no
MAGIC_ENABLE = no  # Magic Keycodes (NKRO toggle, GUI/ALT swap, etc.) - disabled to save space

# Enable MIDI features for piano keyboard
MIDI_ENABLE = yes
# MIDI_ADVANCED is defined in config.h (required for keymap MIDI keycodes)

# Combos disabled (was used for Artsey layout)
COMBO_ENABLE = no
