# Function Signatures — librdrcommon.a (RT82)

## Overview

The RT82 `librdrcommon.a` contains **118 code functions** (T symbols), **~130 data/BSS variables**, and depends on **60 external symbols** from QMK, ChibiOS, and the ES (EastSoft) MCU SDK.

Compiled with **GCC 10.1.0** on Windows (`c:\qmk_msys\...`), targeting ARM Cortex-M0 (Thumb-1 ISA, ELF32 LE, EABI v5).

## Function Categories

### 1. Display / LVGL / LCD (RT82-Only)

These functions are **unique to the RT82** — neither RD75 nor Evo80 include them. This is because the RT82 has a 240x135 RGB565 LCD screen.

| Function | Size (bytes) | Signature (from header) |
|---|---|---|
| `Lvgl_Init` | 252 | `void Lvgl_Init(void)` |
| `Lvgl_Boot` | — | `void Lvgl_Boot(void)` (inferred from keycode) |
| `Reset_Lvgl_Boot` | — | `void Reset_Lvgl_Boot(void)` (inferred) |
| `Reset_Power_Lvgl` | — | `void Reset_Power_Lvgl(void)` |
| `App_Uart_Main_Loop` | 448 | `void App_Uart_Main_Loop(void)` |
| `Init_Uart_Information` | — | `void Init_Uart_Information(void)` |
| `Uart_Dma_Init` | 168 | `void Uart_Dma_Init(void)` |
| `Uart_Tx_Dma_Start` | — | `void Uart_Tx_Dma_Start(void)` |
| `Uart_Rx_Dma_Start` | — | `void Uart_Rx_Dma_Start(void)` |
| `User_Uart_Send_Data` | 304 | `uint8_t User_Uart_Send_Data(uint8_t Command, uint8_t Busy)` |
| `Uart_While_Send_Commad` | — | `void Uart_While_Send_Commad(uint8_t Command, uint8_t Busy)` |
| `Reset_Uart` | — | `void Reset_Uart(void)` |
| `User_Uart_Deinit` | — | `void User_Uart_Deinit(void)` |
| `Scan_Uart_Rf` | 332 | `void Scan_Uart_Rf(void)` |
| `Scan_Uart_Work_Mode` | 124 | `void Scan_Uart_Work_Mode(void)` |
| `Scan_Uart_Func` | 320 | `void Scan_Uart_Func(void)` |
| `Scan_Uart_Batt` | 148 | `void Scan_Uart_Batt(void)` |
| `Scan_Uart_Gif` | 124 | `void Scan_Uart_Gif(void)` |
| `Get_Lcd_Usb_Work_Status` | — | `uint8_t Get_Lcd_Usb_Work_Status(void)` |
| `Lcd_Usb_Work_Init` | — | `void Lcd_Usb_Work_Init(uint8_t Reset, uint8_t Set)` |
| `Scan_Lcd_Usb_Work_Mode` | — | `void Scan_Lcd_Usb_Work_Mode(void)` |
| `Uart_Key_Mode_Key_Scan` | 140 | `void Uart_Key_Mode_Key_Scan(uint16_t Code, bool Press)` |
| `Lcd_Work_Sync_GIF` | — | `void Lcd_Work_Sync_GIF(uint8_t Gif_Count)` |

**Protocol**: UART with DMA, commands defined in `Custom_Uart_Command` enum (0xC0-0xC8, 0xDD).

### 2. Wireless / SPI Communication (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `Spi_Main_Loop` | — | `void Spi_Main_Loop(void)` |
| `Spi_Send_Commad` | 280 | `void Spi_Send_Commad(uint8_t Commad)` |
| `Spi_Ack_Send_Commad` | — | `uint8_t Spi_Ack_Send_Commad(uint8_t Commad)` |
| `Get_Spi_Return_Data` | 400 | `void Get_Spi_Return_Data(uint8_t *Data)` |
| `es_ble_spi_init` | — | `void es_ble_spi_init(void)` |
| `es_ble_spi_deinit` | — | `void es_ble_spi_deinit(void)` |
| `es_spi_send_recv_by_dma` | 188 | `void es_spi_send_recv_by_dma(uint32_t num, uint8_t *rx_buf, uint8_t *tx_buf)` |
| `Ble_Name_Synchronization` | — | `void Ble_Name_Synchronization(void)` |
| `Mode_Synchronization` | — | `void Mode_Synchronization(void)` |
| `User_bluetooth_send_keyboard` | 184 | `void User_bluetooth_send_keyboard(uint8_t *report, uint32_t len)` |

**Protocol**: SPI2 with DMA (ch2 primary/alternate configs). 64-byte TX/RX buffers. Commands defined as `USER_SWITCH_*` / `USER_KEY_*` / `USER_BATTERY_*` macros.

### 3. Battery / ADC (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `User_Adc_Init` | 140 | `void User_Adc_Init(void)` |
| `User_Adc_Deinit` | — | `void User_Adc_Deinit(void)` |
| `User_Adc_Batt_Number` | 480 | `void User_Adc_Batt_Number(void)` |
| `User_Adc_Batt_Power_Up_Init` | 284 | `void User_Adc_Batt_Power_Up_Init(void)` |
| `Init_Batt_Infomation` | 140 | `void Init_Batt_Infomation(void)` |
| `U16_Buff_Clear` | — | `void U16_Buff_Clear(uint16_t *Buff, uint8_t Len)` |

**Thresholds**: Full=2555 (~4.13V), Low=2090 (~3.32V), Shutdown=1865 (~3.04V). 10-sample averaging.

### 4. EEPROM / Flash Storage (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `eeprom_driver_init` | 516 | `void eeprom_driver_init(void)` |
| `eeprom_write_block` | 300 | `void eeprom_write_block(const void *buf, void *addr, size_t len)` |
| `eeprom_read_block` | — | `void eeprom_read_block(void *buf, const void *addr, size_t len)` (inferred) |
| `eeprom_write_block_user` | 136 | `void eeprom_write_block_user(const void *buf, void *addr, size_t len)` |
| `eeprom_read_block_user` | — | `void eeprom_read_block_user(void *buf, const void *addr, size_t len)` |
| `eeprom_driver_erase` | — | `void eeprom_driver_erase(void)` (inferred) |
| `ee_format` | — | (static) Flash page format |
| `ee_verify_pagefull_write_variable` | — | (static) Flash wear-leveling write |
| `IAPROM_PAGE_ERASE.isra.0` | — | (static) In-Application Programming ROM page erase |
| `IAPROM_WORD_PROGRAM` | — | (static) IAP word program |

Uses EastSoft flash controller: `md_fc_lock`, `md_fc_unlock`, `md_fc_page_erase`, `md_fc_program`. Emulated EEPROM in flash with wear-leveling (`g_es_flash_eeprom_table`).

### 5. RGB Matrix / PWM+DMA LED Driver (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `rgb_matrix_driver_init` | 176 | `void rgb_matrix_driver_init(void)` |
| `rgb_matrix_driver_flush` | — | `void rgb_matrix_driver_flush(void)` |
| `rgb_matrix_driver_flush_pwm_dma_start` | 300 | `void rgb_matrix_driver_flush_pwm_dma_start(void)` |
| `rgb_matrix_driver_set_color` | 196 | `void rgb_matrix_driver_set_color(int index, uint8_t r, uint8_t g, uint8_t b)` |
| `rgb_matrix_driver_set_color_all` | — | `void rgb_matrix_driver_set_color_all(uint8_t r, uint8_t g, uint8_t b)` |
| `User_Pwm_Deinit` | — | `void User_Pwm_Deinit(void)` |

**Implementation**: WS2812 protocol via PWM+DMA. 24 bits per LED, H=43, L=17 timer ticks. DMA buffer = 82 LEDs × 24 bits + 2.

### 6. LED Effects / Status Indicators (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `User_Led_Show` | 492 | `void User_Led_Show(void)` |
| `Led_Rf_Mode_Show` | 228 | `void Led_Rf_Mode_Show(void)` |
| `Led_Batt_Number_Show` | 220 | `void Led_Batt_Number_Show(void)` |
| `Led_Point_Flash_Show` | 372 | `void Led_Point_Flash_Show(void)` |
| `Led_Power_Low_Show` | — | `void Led_Power_Low_Show(void)` |
| `User_Get_Led_Power_Status` | — | `void User_Get_Led_Power_Status(void)` |
| `User_Test_Colour_Show` | — | `void User_Test_Colour_Show(void)` |

### 7. Keyboard Init / System Management (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `User_Keyboard_Init` | 232 | `void User_Keyboard_Init(void)` |
| `User_Keyboard_Post_Init` | — | `void User_Keyboard_Post_Init(void)` |
| `User_Keyboard_Reset` | 160 | `void User_Keyboard_Reset(void)` |
| `Init_Keyboard_Infomation` | 348 | `void Init_Keyboard_Infomation(void)` |
| `Init_Gpio_Infomation` | 228 | `void Init_Gpio_Infomation(void)` |
| `User_Systime_Init` | — | `void User_Systime_Init(void)` |
| `User_Systime_Deinit` | — | `void User_Systime_Deinit(void)` |
| `Save_Flash_Set` | — | `void Save_Flash_Set(void)` |
| `es_mcu_reset` | — | `void es_mcu_reset(void)` |
| `mcu_reset` | — | `void mcu_reset(void)` |
| `bootloader_jump` | 128 | `void bootloader_jump(void)` |

### 8. Sleep / Wake / Power Management (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `User_Sleep` | — | `void User_Sleep(void)` |
| `User_Wakeup` | — | `void User_Wakeup(void)` |
| `Board_Wakeup_Init` | 252 | `void Board_Wakeup_Init(void)` |
| `es_chibios_user_idle_loop_hook` | **1484** | `void es_chibios_user_idle_loop_hook(void)` |
| `Sleep_Time_Synchronization` | — | `void Sleep_Time_Synchronization(void)` (RT82-only) |
| `DSleep_Time_Synchronization` | — | `void DSleep_Time_Synchronization(void)` (RT82-only) |

**Note**: `es_chibios_user_idle_loop_hook` is the **largest function** at 1,484 bytes — this is the main system tick loop handling mode switching, sleep timers, LED updates, SPI polling, and battery monitoring.

### 9. USB Management (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `User_Usb_Init` | — | `void User_Usb_Init(void)` |
| `User_Usb_Deinit` | — | `void User_Usb_Deinit(void)` |
| `es_restart_usb_driver` | — | `void es_restart_usb_driver(void)` |
| `Usb_Disconnect` | — | `void Usb_Disconnect(void)` |

### 10. Key Report / HID (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `User_send_6kro_report` | — | `void User_send_6kro_report(void)` |
| `User_send_nkro_report` | — | `void User_send_nkro_report(void)` |
| `User_Clear_Board` | — | `void User_Clear_Board(void)` |
| `add_key_to_report` | — | `void add_key_to_report(uint8_t key)` (RT82+Evo80) |
| `del_key_from_report` | 116 | `void del_key_from_report(uint8_t key)` (RT82+Evo80) |
| `General_Key_Reorder` | — | `void General_Key_Reorder(uint8_t Spot_Index)` (RT82+Evo80) |
| `es_keyboard_leds` | — | `uint8_t es_keyboard_leds(void)` |
| `es_send_keyboard` | — | `void es_send_keyboard(report_keyboard_t *report)` |
| `es_send_nkro` | — | `void es_send_nkro(report_nkro_t *report)` |
| `es_send_mouse` | — | `void es_send_mouse(report_mouse_t *report)` |
| `es_send_extra` | — | `void es_send_extra(report_extra_t *report)` |
| `es_change_qmk_nkro_mode_enable` | — | `void es_change_qmk_nkro_mode_enable(void)` |
| `es_change_qmk_nkro_mode_disable` | — | `void es_change_qmk_nkro_mode_disable(void)` |
| `User_process_record_user` | **1460** | `bool User_process_record_user(uint16_t keycode, keyrecord_t *record)` (RT82-only) |

### 11. EMI Test (Shared)

| Function | Size (bytes) | Signature |
|---|---|---|
| `Emi_Init` | — | `void Emi_Init(void)` |
| `Emi_Read_Data` | — | `void Emi_Read_Data(uint8_t *User_Data, uint8_t User_Length)` |
| `Emi_Write_Data` | — | `void Emi_Write_Data(uint8_t *User_Data, uint8_t User_Length)` |

### 12. Interrupt Vectors (Shared)

| Vector | Notes |
|---|---|
| `Vector4C` | IRQ handler |
| `Vector54` | IRQ handler |
| `Vector58` | IRQ handler |
| `Vector5C` | 216 bytes (RT82) — likely SPI/DMA complete |
| `Vector64` | RT82-only — likely UART TX DMA |
| `Vector68` | IRQ handler |
| `Vector6C` | RT82-only — likely UART RX DMA |
| `Vector78` | **1400 bytes** — likely Timer/SysTick main handler |
| `Vector7C` | IRQ handler |
| `Vector80` | IRQ handler |
| `Vector84` | IRQ handler |

## External Dependencies

The binary requires these symbols from QMK/ChibiOS/MCU SDK at link time:

### QMK Core
- `keyboard_report`, `keyboard_protocol`, `keymap_config`, `layer_state`, `layer_on`
- `register_code`, `unregister_code`, `clear_keyboard`, `clear_mods`, `clear_weak_mods`
- `real_mods`, `weak_mods`, `nkro_report`, `biton`
- `host_get_driver`, `host_set_driver`, `host_keyboard_send`, `host_nkro_send`, `host_consumer_send`, `host_keyboard_led_state`
- `rgb_matrix_*` (get_hue, get_sat, get_speed, get_val, is_enabled, set_color, sethsv_noeeprom)
- `dynamic_keymap_get_keycode`, `eeconfig_disable`, `soft_reset_keyboard`
- `raw_hid_send`, `timer_read`, `timer_elapsed`

### ChibiOS
- `USBD1` (USB driver instance)
- `chThdSleep` (thread sleep)
- `__port_irq_epilogue` (interrupt epilogue)
- `_pal_lld_setgroupmode` (PAL GPIO configuration)

### EastSoft MCU SDK (`md_*` / `ald_*`)
- `md_gpio_init`, `md_spi_init`, `md_uart_init`, `md_adc_init`, `md_adc_calibration`
- `md_fc_lock`, `md_fc_unlock`, `md_fc_page_erase`, `md_fc_program` (flash controller)
- `md_dma_set_channel_data_start_address_and_length`
- `ald_usb_dev_connect`, `ald_usb_dev_disconnect`, `ald_usb_device_components_init`
- `ald_usb_int_register`, `ald_usb_int_unregister`

### C Runtime
- `memcpy`, `memset`, `memcmp`
- `__aeabi_idiv`, `__aeabi_uidiv` (integer division)
- `__gnu_thumb1_case_sqi`, `__gnu_thumb1_case_uhi`, `__gnu_thumb1_case_uqi` (switch table helpers)
