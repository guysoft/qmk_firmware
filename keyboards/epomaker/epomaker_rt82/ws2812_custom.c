/*
 * WS2812 PWM+DMA driver for ES32/FS026
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
 * Custom WS2812 driver using the ChibiOS PWM HAL (GP16C2T1 timer, channel 1)
 * and the ES32 DMA helper (DMAv1/es32_dma.c).  This replaces the bitbang
 * driver with hardware-driven LED output, freeing the CPU during transfers.
 *
 * QMK selects this file when keyboard.json has:
 *   "ws2812": { "driver": "custom" }
 *
 * The driver implements the standard QMK ws2812 interface:
 *   ws2812_init(), ws2812_set_color(), ws2812_set_color_all(), ws2812_flush()
 *
 * Because the ES32 DMA controller (ARM PL230 µDMA) has a 1024-transfer limit
 * per descriptor, a full frame (82 LEDs × 24 bits + 224 reset = 2192 entries)
 * is sent using DMA PING-PONG mode.  Primary and alternate descriptors are
 * pre-loaded (batches 1 & 2), and when the primary completes the DMA
 * seamlessly switches to the alternate — the timer never stops, so no
 * spurious PWM cycles are inserted between batches.  An ISR reprograms
 * the exhausted descriptor for batch 3 while the other descriptor runs.
 */

#include "ws2812.h"
#include "gpio.h"
#include "hal.h"
#include "es32_dma.h"

/* ====================================================================== */
/*  Configuration defaults (overridable in config.h)                      */
/* ====================================================================== */

#ifndef WS2812_PWM_DRIVER
#    define WS2812_PWM_DRIVER PWM_GP16C2T1
#endif
#ifndef WS2812_PWM_CHANNEL
#    define WS2812_PWM_CHANNEL 1 /* 1-based: maps to CCVAL1 */
#endif
#ifndef WS2812_PWM_PAL_MODE
#    define WS2812_PWM_PAL_MODE 5 /* AF5 for PA2 → GP16C2T1_CH1 */
#endif
#ifndef WS2812_DMA_CHANNEL
#    define WS2812_DMA_CHANNEL ES32_DMA_CHANNEL_3
#endif
#ifndef WS2812_DMA_PERIPH_REQ
#    define WS2812_DMA_PERIPH_REQ 66 /* MD_DMA_PRS_GP16C2T1_UP */
#endif

/* ====================================================================== */
/*  Timing constants                                                      */
/* ====================================================================== */

/*
 * The timer clock is the APB bus clock.  With the default PLL0 config in
 * hal_lld.c (72 MHz PLL, HPRE=1, PPRE=1) this is 72 MHz.
 */
#define WS2812_TIMER_CLOCK 72000000UL

/* WS2812 bit rate: 800 kHz (1.25 µs per bit) */
#ifndef WS2812_PWM_FREQUENCY
#    define WS2812_PWM_FREQUENCY (1000000000UL / WS2812_TIMING)
#endif

/* PWM period in timer ticks:  72 MHz / 800 kHz = 90 */
#define WS2812_PWM_PERIOD (WS2812_TIMER_CLOCK / WS2812_PWM_FREQUENCY)

/* Duty cycle (timer ticks) for a logical 0:  72 MHz × 350 ns ≈ 25 */
#define WS2812_DUTYCYCLE_0 (WS2812_TIMER_CLOCK / (1000000000UL / WS2812_T0H))

/* Duty cycle (timer ticks) for a logical 1:  72 MHz × 900 ns ≈ 64 */
#define WS2812_DUTYCYCLE_1 (WS2812_TIMER_CLOCK / (1000000000UL / WS2812_T1H))

/* ====================================================================== */
/*  Frame buffer sizing                                                   */
/* ====================================================================== */

#ifdef WS2812_RGBW
#    define WS2812_CHANNELS 4
#else
#    define WS2812_CHANNELS 3
#endif

#define WS2812_COLOR_BITS (WS2812_CHANNELS * 8)
#define WS2812_RESET_BIT_N (1000 * WS2812_TRST_US / WS2812_TIMING)
#define WS2812_COLOR_BIT_N (WS2812_LED_COUNT * WS2812_COLOR_BITS)
#define WS2812_BIT_N (WS2812_COLOR_BIT_N + WS2812_RESET_BIT_N)

/* DMA batch sizes (max 1024 per PL230 descriptor) */
#define DMA_MAX_XFER 1024

#if WS2812_BIT_N <= DMA_MAX_XFER
#    define DMA_BATCH_COUNT 1
#    define DMA_BATCH_1 WS2812_BIT_N
#    define DMA_BATCH_2 0
#    define DMA_BATCH_3 0
#elif WS2812_BIT_N <= (DMA_MAX_XFER * 2)
#    define DMA_BATCH_COUNT 2
#    define DMA_BATCH_1 DMA_MAX_XFER
#    define DMA_BATCH_2 (WS2812_BIT_N - DMA_MAX_XFER)
#    define DMA_BATCH_3 0
#elif WS2812_BIT_N <= (DMA_MAX_XFER * 3)
#    define DMA_BATCH_COUNT 3
#    define DMA_BATCH_1 DMA_MAX_XFER
#    define DMA_BATCH_2 DMA_MAX_XFER
#    define DMA_BATCH_3 (WS2812_BIT_N - DMA_MAX_XFER * 2)
#else
#    error "WS2812 frame too large for three DMA batches"
#endif

/* Sanity checks */
#if WS2812_DUTYCYCLE_0 > 90 || WS2812_DUTYCYCLE_1 > 90
#    error "WS2812 duty cycle exceeds PWM period"
#endif

/* ====================================================================== */
/*  Bit-index macros (GRB byte order by default)                          */
/* ====================================================================== */

#define WS2812_BIT(led, byte, bit) (WS2812_COLOR_BITS * (led) + 8 * (byte) + (7 - (bit)))

#if (WS2812_BYTE_ORDER == WS2812_BYTE_ORDER_GRB)
#    define WS2812_RED_BIT(led, bit) WS2812_BIT((led), 1, (bit))
#    define WS2812_GREEN_BIT(led, bit) WS2812_BIT((led), 0, (bit))
#    define WS2812_BLUE_BIT(led, bit) WS2812_BIT((led), 2, (bit))
#elif (WS2812_BYTE_ORDER == WS2812_BYTE_ORDER_RGB)
#    define WS2812_RED_BIT(led, bit) WS2812_BIT((led), 0, (bit))
#    define WS2812_GREEN_BIT(led, bit) WS2812_BIT((led), 1, (bit))
#    define WS2812_BLUE_BIT(led, bit) WS2812_BIT((led), 2, (bit))
#elif (WS2812_BYTE_ORDER == WS2812_BYTE_ORDER_BGR)
#    define WS2812_RED_BIT(led, bit) WS2812_BIT((led), 2, (bit))
#    define WS2812_GREEN_BIT(led, bit) WS2812_BIT((led), 1, (bit))
#    define WS2812_BLUE_BIT(led, bit) WS2812_BIT((led), 0, (bit))
#endif

#ifdef WS2812_RGBW
#    define WS2812_WHITE_BIT(led, bit) WS2812_BIT((led), 3, (bit))
#endif

/* ====================================================================== */
/*  Private data                                                          */
/* ====================================================================== */

/*
 * Frame buffer: one uint16_t per bit-period.  Each entry holds a PWM duty
 * cycle value that the DMA writes to the timer's CCVAL1 register.
 *
 * Using uint16_t because GP16C2T1 is a 16-bit timer; DMA transfers as
 * half-word to half-word.
 */
uint16_t ws2812_frame_buffer[WS2812_BIT_N + 1];
const uint32_t ws2812_frame_buffer_size = WS2812_BIT_N;

/* LED colour array — written by ws2812_set_color(), flushed by ws2812_flush() */
static ws2812_led_t ws2812_leds[WS2812_LED_COUNT];

/* Transfer state */
static volatile bool ws2812_xfer_done  = true;
static volatile uint8_t ws2812_xfer_phase = 0; /* 0 = idle, 1..DMA_BATCH_COUNT = active batch */

/* ====================================================================== */
/*  ISR timing instrumentation (exposed to ws2812_debug.c)                */
/* ====================================================================== */

/*
 * On ISR entry we snapshot GP16C2T1->COUNT.  Since the timer period is
 * WS2812_PWM_PERIOD (90) ticks, COUNT tells us how far into the current
 * cycle the ISR fired.  A value close to 90 means the DMA completed near
 * the end of the previous cycle and the ISR fired almost immediately;
 * a value close to 0 means the ISR was delayed by nearly a full period.
 * Values above 90 would indicate multiple missed periods, but COUNT wraps
 * at REPAR so we can only measure intra-period latency directly.
 */
volatile uint32_t ws2812_isr_count_snapshot = 0;
volatile uint32_t ws2812_isr_latency_min    = 0xFFFFFFFF;
volatile uint32_t ws2812_isr_latency_max    = 0;
volatile uint32_t ws2812_isr_latency_last   = 0;
volatile uint32_t ws2812_isr_call_count     = 0;

/* ====================================================================== */
/*  Helpers                                                               */
/* ====================================================================== */

/**
 * @brief  Build a DMA config word for a batch.
 *
 * @param  count  Number of transfers in this batch (≤ 1024).
 * @param  mode   DMA cycle mode (BASIC, PING_PONG, etc.).
 * @return        The config word.
 */
static uint32_t make_dma_cfg(uint32_t count, uint32_t mode) {
    return ES32_DMA_CFG_WORD(
        mode,
        count - 1,                               /* n_minus_1                  */
        MD_DMA_CHANNEL_CFG_RPOWER_SIZE_1,        /* Arbitrate every transfer   */
        MD_DMA_CHANNEL_CFG_SRCDATA_SIZE_HALF_WORD,
        MD_DMA_CHANNEL_CFG_SRCINC_HALF_WORD,
        MD_DMA_CHANNEL_CFG_DSTDATA_SIZE_HALF_WORD,
        MD_DMA_CHANNEL_CFG_DSTINC_NO_INC);
}

/**
 * @brief  Compute the PL230 source-end address for a frame-buffer batch.
 *
 * PL230 µDMA source/destination "end" addresses point to the LAST byte
 * of the transfer, not the first.  For half-word (2-byte) elements the
 * end address = start + (count - 1) * 2.
 */
static uint32_t batch_src_end(const uint16_t *buf_start, uint32_t count) {
    return (uint32_t)buf_start + (count - 1) * sizeof(uint16_t);
}

/**
 * @brief  Stop timer and zero CCVAL1.
 *
 * With CH1 preload disabled (see ws2812_init), writes to CCVAL1 go
 * directly to the active compare register — no shadow/preload pipeline.
 * We stop the timer and zero the register, then reset COUNT via a
 * software update event so the next frame starts cleanly.
 */
static void stop_timer_and_zero_output(void) {
    CLEAR_BIT(GP16C2T1->CON1, TIMER_CON1_CNTEN);
    GP16C2T1->CCVAL1 = 0;
    /* Reset COUNT to 0 via software update event.
     * Temporarily disable DMA-on-update so this doesn't trigger a
     * spurious DMA transfer. */
    GP16C2T1->DMAEN  = 0;
    GP16C2T1->SGE    = TIMER_SGE_SGUPD;
    GP16C2T1->ICR    = TIMER_ICR_UPD;
    GP16C2T1->DMAEN  = TIMER_DMAEN_UPD_MSK;
}

/* ====================================================================== */
/*  DMA completion callback (runs in ISR context)                         */
/* ====================================================================== */

/*
 * PING-PONG mode ISR flow (3 batches):
 *
 *   ws2812_flush():
 *     Primary  = batch 1 (PING_PONG)
 *     Alternate = batch 2 (PING_PONG)
 *     Enable channel, start timer.  Phase = 1.
 *
 *   ISR #1 — primary (batch 1) done:
 *     DMA has already switched to alternate (batch 2 is running).
 *     Reprogram primary → batch 3 (BASIC mode, it's the last batch).
 *     Phase = 2.
 *
 *   ISR #2 — alternate (batch 2) done:
 *     DMA has already switched to primary (batch 3 is running).
 *     Nothing to reprogram.  Phase = 3.
 *
 *   ISR #3 — primary (batch 3) done:
 *     Transfer complete.  Stop timer, zero output.  Phase = 0.
 */

static void dma_complete_cb(void *p, uint32_t flags) {
    (void)p;

    if (!(flags & ES32_DMA_ISR_DONE)) {
        return;
    }

    /* ---- ISR latency measurement ---- */
    {
        uint32_t cnt = GP16C2T1->COUNT;
        ws2812_isr_count_snapshot = cnt;
        ws2812_isr_latency_last  = cnt;
        if (cnt < ws2812_isr_latency_min) ws2812_isr_latency_min = cnt;
        if (cnt > ws2812_isr_latency_max) ws2812_isr_latency_max = cnt;
        ws2812_isr_call_count++;
    }

#if DMA_BATCH_COUNT == 1
    /* Single batch — just stop. */
    stop_timer_and_zero_output();
    ws2812_xfer_phase = 0;
    ws2812_xfer_done  = true;

#elif DMA_BATCH_COUNT == 2
    /*
     * 2-batch PING-PONG:
     *   Phase 1 (primary/batch 1 done) → alternate/batch 2 is already running.
     *   Phase 2 (alternate/batch 2 done) → transfer complete.
     */
    if (ws2812_xfer_phase == 1) {
        /* Batch 2 is already running on the alternate descriptor.
         * Nothing to reprogram — just advance the phase. */
        ws2812_xfer_phase = 2;
    } else {
        stop_timer_and_zero_output();
        ws2812_xfer_phase = 0;
        ws2812_xfer_done  = true;
    }

#elif DMA_BATCH_COUNT == 3
    /*
     * 3-batch PING-PONG:
     *   Phase 1 (primary/batch 1 done)   → reprogram primary with batch 3.
     *   Phase 2 (alternate/batch 2 done) → batch 3 already running on primary.
     *   Phase 3 (primary/batch 3 done)   → transfer complete.
     */
    if (ws2812_xfer_phase == 1) {
        /* Alternate (batch 2) is already running.  Reprogram primary
         * descriptor for batch 3.  Use BASIC mode (no further ping-pong). */
        uint32_t dst_end = (uint32_t)&GP16C2T1->CCVAL1;
        uint32_t src_end = batch_src_end(
            &ws2812_frame_buffer[DMA_BATCH_1 + DMA_BATCH_2], DMA_BATCH_3);
        uint32_t cfg = make_dma_cfg(DMA_BATCH_3,
                                     MD_DMA_CHANNEL_CFG_MODE_BASIC);
        es32_dma_channel_reprogram_primary(WS2812_DMA_CHANNEL,
                                           src_end, dst_end, cfg);
        ws2812_xfer_phase = 2;
    } else if (ws2812_xfer_phase == 2) {
        /* Primary (batch 3) is already running.  Nothing to do. */
        ws2812_xfer_phase = 3;
    } else {
        /* Batch 3 done — transfer complete. */
        stop_timer_and_zero_output();
        ws2812_xfer_phase = 0;
        ws2812_xfer_done  = true;
    }
#endif
}

/* ====================================================================== */
/*  Public API (QMK ws2812 interface)                                     */
/* ====================================================================== */

void ws2812_init(void) {
    /* ---- Initialise frame buffer ---- */
    for (uint32_t i = 0; i < WS2812_COLOR_BIT_N; i++) {
        ws2812_frame_buffer[i] = WS2812_DUTYCYCLE_0;
    }
    for (uint32_t i = 0; i < WS2812_RESET_BIT_N; i++) {
        ws2812_frame_buffer[i + WS2812_COLOR_BIT_N] = 0;
    }
    ws2812_frame_buffer[WS2812_BIT_N] = 0; /* guard element */

    /* ---- Configure PA2 as alternate function for GP16C2T1_CH1 ---- */
    palSetLineMode(WS2812_DI_PIN,
                   PAL_MODE_ALTERNATE(WS2812_PWM_PAL_MODE));

    /* ---- Allocate a DMA channel ---- */
    es32_dma_channel_alloc(WS2812_DMA_CHANNEL, 3, dma_complete_cb, NULL);

    /* ---- Configure & start PWM ---- */
    static const PWMConfig ws2812_pwm_config = {
        .frequency = WS2812_TIMER_CLOCK, /* Prescaler = 0 (no division)  */
        .period    = WS2812_PWM_PERIOD,  /* 90 ticks → 800 kHz           */
        .callback  = NULL,
        .channels  = {
            {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL}, /* CH1 (0-idx 0) */
            {.mode = PWM_OUTPUT_DISABLED,    .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED,    .callback = NULL},
            {.mode = PWM_OUTPUT_DISABLED,    .callback = NULL},
        },
        .con2  = 0,
        .bdcfg = 0,
        .dmaen = TIMER_DMAEN_UPD_MSK, /* DMA request on update (overflow) */
    };

    pwmStart(&WS2812_PWM_DRIVER, &ws2812_pwm_config);

    /*
     * CRITICAL: Disable CH1 compare-register preload.
     *
     * pwmStart() calls the LLD which enables preload (ch1pen) on all
     * channels.  With preload ON, DMA writes go to a preload register
     * and only reach the active compare register at the NEXT update
     * event — a 1-cycle pipeline delay.  This means the LAST DMA
     * transfer of each batch is written to preload but never reaches
     * the active register (because the ISR stops the timer before the
     * next update event), effectively dropping 1 bit at every batch
     * boundary and shifting all subsequent LED colour data.
     *
     * With preload OFF, DMA writes go directly to the active compare
     * register, taking effect in the SAME cycle.  No pipeline delay,
     * no lost bits.
     */
    CLEAR_BIT(GP16C2T1->CHMR1, TIMER_CHMR1_OUTPUT_CH1PEN);

    /* Initial duty = 0 → output stays low until first DMA transfer */
    pwmEnableChannel(&WS2812_PWM_DRIVER, WS2812_PWM_CHANNEL - 1, 0);
}

void ws2812_set_color(int index, uint8_t red, uint8_t green, uint8_t blue) {
    ws2812_leds[index].r = red;
    ws2812_leds[index].g = green;
    ws2812_leds[index].b = blue;
#if defined(WS2812_RGBW)
    ws2812_rgb_to_rgbw(&ws2812_leds[index]);
#endif
}

void ws2812_set_color_all(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < WS2812_LED_COUNT; i++) {
        ws2812_set_color(i, red, green, blue);
    }
}

void ws2812_flush(void) {
    /* Wait for any in-progress transfer to finish */
    while (!ws2812_xfer_done) {
        __NOP();
    }

    /* ---- Convert LED colours into duty-cycle frame buffer ---- */
    for (int i = 0; i < WS2812_LED_COUNT; i++) {
        for (uint8_t bit = 0; bit < 8; bit++) {
            ws2812_frame_buffer[WS2812_RED_BIT(i, bit)]   = ((ws2812_leds[i].r >> bit) & 0x01) ? WS2812_DUTYCYCLE_1 : WS2812_DUTYCYCLE_0;
            ws2812_frame_buffer[WS2812_GREEN_BIT(i, bit)] = ((ws2812_leds[i].g >> bit) & 0x01) ? WS2812_DUTYCYCLE_1 : WS2812_DUTYCYCLE_0;
            ws2812_frame_buffer[WS2812_BLUE_BIT(i, bit)]  = ((ws2812_leds[i].b >> bit) & 0x01) ? WS2812_DUTYCYCLE_1 : WS2812_DUTYCYCLE_0;
#ifdef WS2812_RGBW
            ws2812_frame_buffer[WS2812_WHITE_BIT(i, bit)] = ((ws2812_leds[i].w >> bit) & 0x01) ? WS2812_DUTYCYCLE_1 : WS2812_DUTYCYCLE_0;
#endif
        }
    }

    /* ---- Ensure timer is stopped and CCVAL1 = 0 before starting ---- */
    stop_timer_and_zero_output();

    ws2812_xfer_done  = false;
    ws2812_xfer_phase = 1;

    uint32_t dst_end = (uint32_t)&GP16C2T1->CCVAL1;

#if DMA_BATCH_COUNT == 1
    /* ---- Single batch: plain BASIC mode ---- */
    {
        uint32_t src_end = batch_src_end(&ws2812_frame_buffer[0], DMA_BATCH_1);
        uint32_t cfg = make_dma_cfg(DMA_BATCH_1,
                                     MD_DMA_CHANNEL_CFG_MODE_BASIC);
        es32_dma_channel_setup(WS2812_DMA_CHANNEL,
                               WS2812_DMA_PERIPH_REQ,
                               src_end, dst_end, cfg);
        es32_dma_channel_enable(WS2812_DMA_CHANNEL);
    }

#elif DMA_BATCH_COUNT >= 2
    /* ---- PING-PONG mode: primary = batch 1, alternate = batch 2 ---- */
    {
        /* Primary descriptor (batch 1) — PING_PONG mode */
        uint32_t src1 = batch_src_end(&ws2812_frame_buffer[0], DMA_BATCH_1);
        uint32_t cfg1 = make_dma_cfg(DMA_BATCH_1,
                                      MD_DMA_CHANNEL_CFG_MODE_PING_PONG);
        es32_dma_channel_setup(WS2812_DMA_CHANNEL,
                               WS2812_DMA_PERIPH_REQ,
                               src1, dst_end, cfg1);

        /* Alternate descriptor (batch 2) — PING_PONG mode */
        uint32_t src2 = batch_src_end(&ws2812_frame_buffer[DMA_BATCH_1],
                                       DMA_BATCH_2);
#if DMA_BATCH_COUNT == 2
        /* Last batch on alternate — use BASIC so DMA stops after it. */
        uint32_t cfg2 = make_dma_cfg(DMA_BATCH_2,
                                      MD_DMA_CHANNEL_CFG_MODE_BASIC);
#else
        /* More batches to come — use PING_PONG so DMA switches back. */
        uint32_t cfg2 = make_dma_cfg(DMA_BATCH_2,
                                      MD_DMA_CHANNEL_CFG_MODE_PING_PONG);
#endif
        es32_dma_channel_setup_alternate(WS2812_DMA_CHANNEL,
                                          src2, dst_end, cfg2);

        es32_dma_channel_enable(WS2812_DMA_CHANNEL);
    }
#endif

    /* ---- Start the timer ---- */
    SET_BIT(GP16C2T1->CON1, TIMER_CON1_CNTEN);
}

uint32_t get_ws2812_frame_buffer_size(void) {
    return WS2812_BIT_N + 1;
}
