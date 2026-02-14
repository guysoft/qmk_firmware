/* Copyright 2025 Guy
 *
 * RT82 Screen Driver - UART4 communication with the display controller.
 *
 * The RT82 display is a separate controller module (240x135 RGB565) connected
 * to the ES32F0283 keyboard MCU via:
 *   - UART4 at 115200 baud, 8N1 (PA0 = TX AF6, PA1 = RX AF6)
 *   - PA5 = Reset (active low)
 *   - PA6 = Boot
 *   - PA8 = Power
 *
 * Communication uses 8-byte packets:
 *   [0x11] [0x22] [0x33] [0x44] [CMD] [DATA1] [DATA2] [0x00]
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/* ---- Screen commands (from original firmware rdr_common.h) ------------- */
#define SCREEN_CMD_WORK_MODE      0xC0  /* Work mode (home/key/gif/test) */
#define SCREEN_CMD_LED_DATA       0xC1  /* LED indicator data */
#define SCREEN_CMD_BATT_DATA      0xC2  /* Battery level data */
#define SCREEN_CMD_MODE_DATA      0xC3  /* RF mode (BLE/2.4G/USB) */
#define SCREEN_CMD_KEY_MODE_DATA  0xC4  /* Key press data for typing mode */
#define SCREEN_CMD_HOME_MOV_DATA  0xC5  /* Home screen navigation */
#define SCREEN_CMD_GIF_CHANGE     0xC6  /* GIF change / download trigger */
#define SCREEN_CMD_TEST_COLOUR    0xC7  /* Solid test colour */
#define SCREEN_CMD_USB_DATA       0xC8  /* Enable screen USB (0x1919) */
#define SCREEN_CMD_USB_DISABLE    0xDD  /* Disable screen USB */

/* Work modes (DATA1 for 0xC0) */
#define SCREEN_MODE_HOME          0x00
#define SCREEN_MODE_KEY           0x01
#define SCREEN_MODE_GIF           0x02
#define SCREEN_MODE_TEST          0x03

/* USB data values (DATA1 for 0xC8) */
#define SCREEN_USB_ON             0x01
#define SCREEN_USB_OFF            0x00

/* ---- Public API -------------------------------------------------------- */

/**
 * Initialise UART4 peripheral, GPIO pins (PA0/PA1 for UART, PA5/PA6/PA8 for
 * screen control), and enable clocks.  Must be called once at startup before
 * any other screen_*() functions.
 */
void screen_init(void);

/**
 * Execute the screen power-on / boot sequence:
 *   1. PA8 high (power on)
 *   2. Delay
 *   3. PA5 high (deassert reset)
 *   4. PA6 high (boot)
 *   5. Delay
 *   6. Send work-mode Home command
 */
void screen_boot(void);

/**
 * Power off the screen (all control GPIOs low).
 */
void screen_off(void);

/**
 * Send a raw 8-byte screen command packet.
 * @param cmd    Command byte (0xC0..0xC8 or 0xDD)
 * @param data1  First data byte
 * @param data2  Second data byte
 */
void screen_send_cmd(uint8_t cmd, uint8_t data1, uint8_t data2);

/**
 * Set screen work mode.
 * @param mode  One of SCREEN_MODE_HOME, SCREEN_MODE_KEY, etc.
 */
void screen_set_mode(uint8_t mode);

/**
 * Display a solid test colour on the screen.
 * @param colour_id  Colour index (0-7 typical, device-specific)
 */
void screen_test_colour(uint8_t colour_id);

/**
 * Enable the screen controller's USB interface (VID 0x1919 PID 0x1919).
 * This makes the screen's own USB device appear on the host so that
 * rt82display can upload QGIF images.
 */
void screen_enable_usb(void);

/**
 * Disable the screen controller's USB interface.
 */
void screen_disable_usb(void);

/**
 * Cycle to the next screen work mode.
 * Order: Home → GIF → Key → Home …
 * @return The new mode (SCREEN_MODE_HOME, SCREEN_MODE_GIF, or SCREEN_MODE_KEY).
 */
uint8_t screen_next_mode(void);

/**
 * Get the current screen work mode.
 */
uint8_t screen_get_mode(void);

/**
 * Full re-initialisation: power-cycle, re-init UART, re-boot.
 * Equivalent to Fn+C in the factory firmware.
 */
void screen_reset(void);
