/* Copyright 2025 Guy
 *
 * RT82 Screen Driver - bare-metal UART4 + GPIO driver for the display
 * controller module.
 *
 * Uses the ES32 FS026 micro-driver (md_*) functions directly because the
 * ChibiOS serial LLD for ES32 is only a stub and does not support UART4.
 *
 * Pin configuration (from factory firmware disassembly):
 *   PA0 - UART4 TX, AF6, push-pull, no pull
 *   PA1 - UART4 RX, AF6, push-pull, pull-up
 *   PA5 - Screen Reset, GPIO output (active low, deassert = high)
 *   PA6 - Screen Boot,  GPIO output (set high to boot)
 *   PA8 - Screen Power, GPIO output (set high to power on)
 */

#include "rt82_screen.h"
#include "ch.h"
#include "hal.h"

/* ES32 FS026 micro-driver headers */
#include "md_uart.h"
#include "md_gpio.h"
#include "md_rcu.h"

/* ---- Mode tracking (must precede screen_set_mode) ---------------------- */
static uint8_t current_mode = SCREEN_MODE_HOME;

/* ---- Packet format ----------------------------------------------------- */
#define PKT_HEADER_0  0x11
#define PKT_HEADER_1  0x22
#define PKT_HEADER_2  0x33
#define PKT_HEADER_3  0x44
#define PKT_SIZE      8

/* ---- Internal helpers -------------------------------------------------- */

/**
 * Busy-wait until the UART4 TX FIFO is empty, then send one byte.
 */
static void uart4_putc(uint8_t c) {
    /* Wait for TX FIFO empty */
    while (!md_uart_is_active_flag_tfempty(UART4)) {
        /* spin */
    }
    md_uart_set_send_data8(UART4, c);
}

/**
 * Send a complete 8-byte packet over UART4 using polling TX.
 */
static void uart4_send_packet(uint8_t cmd, uint8_t data1, uint8_t data2) {
    uart4_putc(PKT_HEADER_0);
    uart4_putc(PKT_HEADER_1);
    uart4_putc(PKT_HEADER_2);
    uart4_putc(PKT_HEADER_3);
    uart4_putc(cmd);
    uart4_putc(data1);
    uart4_putc(data2);
    uart4_putc(0x00);
}

/* ---- GPIO helpers ------------------------------------------------------ */

static void screen_gpio_init_output(uint32_t pin_mask) {
    md_gpio_inittypedef gpio_cfg;
    gpio_cfg.Pin       = pin_mask;
    gpio_cfg.Mode      = MD_GPIO_MODE_OUTPUT;
    gpio_cfg.OutputType = MD_GPIO_OUTPUT_PUSHPULL;
    gpio_cfg.Pull      = MD_GPIO_PULL_FLOATING;
    gpio_cfg.OutDrive  = MD_GPIO_DRIVING_8MA;
    gpio_cfg.Function  = MD_GPIO_AF0;  /* don't care for output mode */
    md_gpio_init(GPIOA, &gpio_cfg);
}

/* ---- Public API -------------------------------------------------------- */

void screen_init(void) {
    /* --- Enable clocks -------------------------------------------------- */
    md_rcu_enable_gpioa(RCU);      /* GPIOA clock (probably already on) */
    md_rcu_enable_uart4(RCU);      /* UART4 clock on APB1 */

    /* --- Configure UART4 TX pin: PA0, AF6 ------------------------------- */
    {
        md_gpio_inittypedef gpio_tx;
        gpio_tx.Pin       = MD_GPIO_PIN_0;
        gpio_tx.Mode      = MD_GPIO_MODE_FUNCTION;
        gpio_tx.OutputType = MD_GPIO_OUTPUT_PUSHPULL;
        gpio_tx.Pull      = MD_GPIO_PULL_FLOATING;
        gpio_tx.OutDrive  = MD_GPIO_DRIVING_8MA;
        gpio_tx.Function  = MD_GPIO_AF6;
        md_gpio_init(GPIOA, &gpio_tx);
    }

    /* --- Configure UART4 RX pin: PA1, AF6, pull-up ---------------------- */
    {
        md_gpio_inittypedef gpio_rx;
        gpio_rx.Pin       = MD_GPIO_PIN_1;
        gpio_rx.Mode      = MD_GPIO_MODE_FUNCTION;
        gpio_rx.OutputType = MD_GPIO_OUTPUT_PUSHPULL;
        gpio_rx.Pull      = MD_GPIO_PULL_UP;
        gpio_rx.OutDrive  = MD_GPIO_DRIVING_8MA;
        gpio_rx.Function  = MD_GPIO_AF6;
        md_gpio_init(GPIOA, &gpio_rx);
    }

    /* --- Initialise UART4: 115200 baud, 8N1, LSB first ----------------- */
    {
        md_uart_init_typedef uart_cfg;
        uart_cfg.BaudRate  = MD_UART_BAUDRATE_115200;
        uart_cfg.BitOrder  = MD_UART_LCON_LSB_FIRST;
        uart_cfg.Parity    = MD_UART_LCON_PS_NONE;
        uart_cfg.StopBits  = MD_UART_LCON_STOP_1;
        uart_cfg.DataWidth = MD_UART_LCON_DLS_8;
        md_uart_init(UART4, &uart_cfg);
    }

    /* Enable TX and RX */
    md_uart_enable_tx(UART4);
    md_uart_enable_rx(UART4);

    /* --- Configure screen control GPIOs as outputs, initially low ------- */
    screen_gpio_init_output(MD_GPIO_PIN_5);   /* PA5 - Reset */
    screen_gpio_init_output(MD_GPIO_PIN_6);   /* PA6 - Boot  */
    screen_gpio_init_output(MD_GPIO_PIN_8);   /* PA8 - Power */

    /* Start with all control lines low (screen off / held in reset) */
    md_gpio_set_pin_low(GPIOA, MD_GPIO_PIN_5 | MD_GPIO_PIN_6 | MD_GPIO_PIN_8);
}

void screen_boot(void) {
    /* 1. Power on */
    md_gpio_set_pin_high(GPIOA, MD_GPIO_PIN_8);
    chThdSleepMilliseconds(200);

    /* 2. Deassert reset */
    md_gpio_set_pin_high(GPIOA, MD_GPIO_PIN_5);
    chThdSleepMilliseconds(50);

    /* 3. Boot */
    md_gpio_set_pin_high(GPIOA, MD_GPIO_PIN_6);
    chThdSleepMilliseconds(200);

    /* 4. Send initial work mode: Home */
    screen_send_cmd(SCREEN_CMD_WORK_MODE, SCREEN_MODE_HOME, 0x00);
}

void screen_off(void) {
    md_gpio_set_pin_low(GPIOA, MD_GPIO_PIN_5 | MD_GPIO_PIN_6 | MD_GPIO_PIN_8);
}

void screen_send_cmd(uint8_t cmd, uint8_t data1, uint8_t data2) {
    uart4_send_packet(cmd, data1, data2);
}

void screen_set_mode(uint8_t mode) {
    current_mode = mode;
    screen_send_cmd(SCREEN_CMD_WORK_MODE, mode, 0x00);
}

void screen_test_colour(uint8_t colour_id) {
    screen_send_cmd(SCREEN_CMD_TEST_COLOUR, colour_id, 0x00);
}

void screen_enable_usb(void) {
    screen_send_cmd(SCREEN_CMD_USB_DATA, SCREEN_USB_ON, 0x00);
}

void screen_disable_usb(void) {
    screen_send_cmd(SCREEN_CMD_USB_DISABLE, 0x00, 0x00);
}

/* ---- Mode tracking ----------------------------------------------------- */

uint8_t screen_get_mode(void) {
    return current_mode;
}

uint8_t screen_next_mode(void) {
    /* Cycle: Home → GIF → Key → Home … */
    switch (current_mode) {
        case SCREEN_MODE_HOME: current_mode = SCREEN_MODE_GIF;  break;
        case SCREEN_MODE_GIF:  current_mode = SCREEN_MODE_KEY;  break;
        default:               current_mode = SCREEN_MODE_HOME; break;
    }
    screen_set_mode(current_mode);
    return current_mode;
}

void screen_reset(void) {
    /* Power off everything */
    screen_off();
    chThdSleepMilliseconds(200);

    /* Re-boot */
    screen_boot();
    current_mode = SCREEN_MODE_HOME;
}
