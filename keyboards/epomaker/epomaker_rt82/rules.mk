# Board: FS026 board definition in lib/chibios-contrib
BOARD = FS026

EEPROM_DRIVER = transient
NO_USB_STARTUP_CHECK = yes

ENCODER_MAP_ENABLE = yes
DEBOUNCE_TYPE = asym_eager_defer_pk

SRC += ws2812_debug.c
SRC += rt82_screen.c
