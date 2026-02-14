/*
 * WS2812 RAW HID debug interface
 *
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
 *
 * ---------------------------------------------------------------------------
 *
 * Provides interactive LED testing and ISR timing diagnostics over
 * RAW HID (VIA command hook).  Uses command ID 0xFE with sub-commands.
 *
 * Host-side tool: ws2812_diag.py
 */

#include "quantum.h"
#include "raw_hid.h"
#include "via.h"
#include "ws2812.h"
#include "rgb_matrix.h"

/* ====================================================================== */
/*  Protocol constants                                                    */
/* ====================================================================== */

#define WS2812_DEBUG_CMD    0xFE

/* Sub-command IDs (byte 1 of the 32-byte packet) */
#define SUBCMD_SET_LED      0x01
#define SUBCMD_SET_RANGE    0x02
#define SUBCMD_SET_ALL      0x03
#define SUBCMD_GET_TIMING   0x04
#define SUBCMD_DUMP_BUFFER  0x05
#define SUBCMD_FREEZE_RGB   0x06

/* ====================================================================== */
/*  External symbols from ws2812_custom.c                                 */
/* ====================================================================== */

extern uint16_t        ws2812_frame_buffer[];
extern const uint32_t  ws2812_frame_buffer_size;

extern volatile uint32_t ws2812_isr_count_snapshot;
extern volatile uint32_t ws2812_isr_latency_min;
extern volatile uint32_t ws2812_isr_latency_max;
extern volatile uint32_t ws2812_isr_latency_last;
extern volatile uint32_t ws2812_isr_call_count;

/* ====================================================================== */
/*  State                                                                 */
/* ====================================================================== */

static bool rgb_frozen = false;

/* ====================================================================== */
/*  Helpers                                                               */
/* ====================================================================== */

static void put_u32_le(uint8_t *buf, uint32_t v) {
    buf[0] = (uint8_t)(v);
    buf[1] = (uint8_t)(v >> 8);
    buf[2] = (uint8_t)(v >> 16);
    buf[3] = (uint8_t)(v >> 24);
}

static uint16_t get_u16_le(const uint8_t *buf) {
    return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

/* ====================================================================== */
/*  Command handlers                                                      */
/* ====================================================================== */

/*
 * SET_LED: [0xFE, 0x01, led_idx, R, G, B]
 * Sets a single LED and flushes.
 */
static void cmd_set_led(uint8_t *data, uint8_t length) {
    uint8_t idx = data[2];
    uint8_t r   = data[3];
    uint8_t g   = data[4];
    uint8_t b   = data[5];

    if (idx < RGB_MATRIX_LED_COUNT) {
        ws2812_set_color(idx, r, g, b);
        ws2812_flush();
    }

    /* ACK */
    data[0] = WS2812_DEBUG_CMD;
    data[1] = SUBCMD_SET_LED;
    data[2] = 0x00; /* OK */
    raw_hid_send(data, length);
}

/*
 * SET_RANGE: [0xFE, 0x02, start, count, R, G, B]
 * Sets a contiguous range of LEDs and flushes.
 */
static void cmd_set_range(uint8_t *data, uint8_t length) {
    uint8_t start = data[2];
    uint8_t count = data[3];
    uint8_t r     = data[4];
    uint8_t g     = data[5];
    uint8_t b     = data[6];

    for (uint8_t i = 0; i < count && (start + i) < RGB_MATRIX_LED_COUNT; i++) {
        ws2812_set_color(start + i, r, g, b);
    }
    ws2812_flush();

    data[0] = WS2812_DEBUG_CMD;
    data[1] = SUBCMD_SET_RANGE;
    data[2] = 0x00;
    raw_hid_send(data, length);
}

/*
 * SET_ALL: [0xFE, 0x03, R, G, B]
 * Sets all LEDs and flushes.
 */
static void cmd_set_all(uint8_t *data, uint8_t length) {
    uint8_t r = data[2];
    uint8_t g = data[3];
    uint8_t b = data[4];

    ws2812_set_color_all(r, g, b);
    ws2812_flush();

    data[0] = WS2812_DEBUG_CMD;
    data[1] = SUBCMD_SET_ALL;
    data[2] = 0x00;
    raw_hid_send(data, length);
}

/*
 * GET_TIMING: [0xFE, 0x04]
 * Response: [0xFE, 0x04, 0x00,
 *            latency_min(4), latency_max(4), latency_last(4),
 *            isr_call_count(4), count_snapshot(4)]
 * All values little-endian uint32.
 */
static void cmd_get_timing(uint8_t *data, uint8_t length) {
    memset(data, 0, length);
    data[0] = WS2812_DEBUG_CMD;
    data[1] = SUBCMD_GET_TIMING;
    data[2] = 0x00;

    put_u32_le(&data[3],  ws2812_isr_latency_min);
    put_u32_le(&data[7],  ws2812_isr_latency_max);
    put_u32_le(&data[11], ws2812_isr_latency_last);
    put_u32_le(&data[15], ws2812_isr_call_count);
    put_u32_le(&data[19], ws2812_isr_count_snapshot);

    /* Reset min/max after read */
    ws2812_isr_latency_min = 0xFFFFFFFF;
    ws2812_isr_latency_max = 0;
    ws2812_isr_call_count  = 0;

    raw_hid_send(data, length);
}

/*
 * DUMP_BUFFER: [0xFE, 0x05, offset_lo, offset_hi, count]
 * Reads `count` uint16_t entries from the frame buffer starting at `offset`.
 * Response: [0xFE, 0x05, 0x00, count, data...]
 * Max count = 14 (to fit 14 × 2 = 28 bytes in the 32-byte packet).
 */
static void cmd_dump_buffer(uint8_t *data, uint8_t length) {
    uint16_t offset = get_u16_le(&data[2]);
    uint8_t  count  = data[4];

    if (count > 14) count = 14;

    memset(data, 0, length);
    data[0] = WS2812_DEBUG_CMD;
    data[1] = SUBCMD_DUMP_BUFFER;
    data[2] = 0x00;
    data[3] = count;

    for (uint8_t i = 0; i < count; i++) {
        uint16_t idx = offset + i;
        uint16_t val = 0;
        if (idx < ws2812_frame_buffer_size + 1) {
            val = ws2812_frame_buffer[idx];
        }
        data[4 + i * 2]     = (uint8_t)(val);
        data[4 + i * 2 + 1] = (uint8_t)(val >> 8);
    }

    raw_hid_send(data, length);
}

/*
 * FREEZE_RGB: [0xFE, 0x06, enable]
 * enable=1: disable RGB matrix updates (freeze current state for manual testing)
 * enable=0: re-enable RGB matrix
 */
static void cmd_freeze_rgb(uint8_t *data, uint8_t length) {
    uint8_t enable = data[2];

    if (enable) {
        rgb_matrix_disable_noeeprom();
        rgb_frozen = true;
    } else {
        rgb_matrix_enable_noeeprom();
        rgb_frozen = false;
    }

    data[0] = WS2812_DEBUG_CMD;
    data[1] = SUBCMD_FREEZE_RGB;
    data[2] = 0x00;
    raw_hid_send(data, length);
}

/* ====================================================================== */
/*  VIA command hook                                                      */
/* ====================================================================== */

bool via_command_kb(uint8_t *data, uint8_t length) {
    if (data[0] != WS2812_DEBUG_CMD) {
        return false; /* Not our command — let VIA handle it */
    }

    switch (data[1]) {
        case SUBCMD_SET_LED:
            cmd_set_led(data, length);
            return true;
        case SUBCMD_SET_RANGE:
            cmd_set_range(data, length);
            return true;
        case SUBCMD_SET_ALL:
            cmd_set_all(data, length);
            return true;
        case SUBCMD_GET_TIMING:
            cmd_get_timing(data, length);
            return true;
        case SUBCMD_DUMP_BUFFER:
            cmd_dump_buffer(data, length);
            return true;
        case SUBCMD_FREEZE_RGB:
            cmd_freeze_rgb(data, length);
            return true;
        default:
            /* Unknown sub-command — NACK */
            data[2] = 0xFF;
            raw_hid_send(data, length);
            return true;
    }
}
