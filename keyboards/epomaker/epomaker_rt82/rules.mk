# Build Options
#   change yes to no to disable
#
# Board: it should exist either in <chibios>/os/hal/boards/
#  or <this_dir>/boards
BOARD = FS026
EEPROM_DRIVER = custom
NO_USB_STARTUP_CHECK = yes
BLUETOOTH_CUSTOM = yes
RAW_ENABLE = yes           # Enable RAW HID for EMI test
VIA_ENABLE = yes           # Enable VIA for dynamic keymap

ENCODER_MAP_ENABLE = yes    # Enable ENCODER MAP
DEBOUNCE_TYPE = asym_eager_defer_pk
