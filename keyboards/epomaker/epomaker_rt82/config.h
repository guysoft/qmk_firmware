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
#pragma once

/* Mechanical locking support. Use KC_LCAP, KC_LNUM or KC_LSCR instead in keymap */
#define LOCKING_SUPPORT_ENABLE
/* Locking resynchronize hack */
#define LOCKING_RESYNC_ENABLE

#define MATRIX_UNSELECT_DRIVE_HIGH
#define CORTEX_ENABLE_WFI_IDLE          FALSE

/* Ensure we jump to bootloader if the RESET keycode was pressed */
#define EARLY_INIT_PERFORM_BOOTLOADER_JUMP TRUE

#define DEBOUNCE 5

#ifndef NOP_FUDGE
#define NOP_FUDGE 0.4
#endif

/* --- RGB Matrix --- */
#define RGB_MATRIX_LED_COUNT 82
#define RGB_MATRIX_KEYPRESSES
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_LED_FLUSH_LIMIT 16

/* --- WS2812 PWM+DMA configuration --- */
#define WS2812_PWM_DRIVER      PWM_GP16C2T1
#define WS2812_PWM_CHANNEL     1              /* 1-based → CCVAL1 register    */
#define WS2812_PWM_PAL_MODE    5              /* AF5: PA2 → GP16C2T1_CH1      */
#define WS2812_DMA_CHANNEL     ES32_DMA_CHANNEL_3
#define WS2812_DMA_PERIPH_REQ  66             /* MD_DMA_PRS_GP16C2T1_UP       */

/* GPIO pin that controls the LED power supply MOSFET */
#define LED_POWER_PIN D0
