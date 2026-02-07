# Binary Comparison Matrix — RT82 vs RD75 vs Evo80

## Binary Overview

| Property | RT82 | RD75 | Evo80 |
|---|---|---|---|
| **Archive Size** | 282,828 bytes | 205,504 bytes | 257,694 bytes |
| **Object File** | `rdr_common.o` | `rdr_common.o` | `rdr_common.o` |
| **.text (code)** | 17,932 bytes | 12,512 bytes | 15,468 bytes |
| **.data (init'd)** | 313 bytes | 954 bytes | 958 bytes |
| **.bss (uninit'd)** | 3,577 bytes | 3,644 bytes | 3,871 bytes |
| **Total in-memory** | 21,822 bytes | 17,110 bytes | 20,297 bytes |
| **ELF Sections** | 408 | 320 | 342 |
| **Code Functions** | 118 | 95 | 109 |
| **Compiler** | GCC 10.1.0 | GCC 10.1.0 | GCC 10.1.0 |
| **Architecture** | ARM ELF32 LE EABI v5 | ARM ELF32 LE EABI v5 | ARM ELF32 LE EABI v5 |
| **Header Size** | 35,660 bytes (909 lines) | 23,069 bytes (578 lines) | 26,351 bytes (652 lines) |

## Feature Comparison

| Feature | RT82 | RD75 | Evo80 |
|---|---|---|---|
| **BLE ID** | `0x008A` | `0x0001` | `0x000C` |
| **BLE Names** | RT82-1/2/3 | RD 75-1/2/3 | EVO80 BT1/2/3 |
| **LCD Display** | Yes (240x135) | No | No |
| **LOGO LEDs** | No (`LOGO_LED_ENABLE=0`) | Yes (`LOGO_LED_ENABLE=1`) | Yes (`LOGO_LED_ENABLE=1`) |
| **Side LEDs** | No (`SIDE_LED_ENABLE=0`) | No (`SIDE_LED_ENABLE=0`) | No (`SIDE_LED_ENABLE=0`) |
| **Encoder** | No (in binary) | Yes | No |
| **UART/DMA (LCD)** | Yes | No | No |
| **Mode Switch** | Yes (physical toggle) | No | No |
| **Deep Sleep** | Yes (2-stage) | No | No |
| **Custom Debounce** | Yes | No | Yes |

## Function Presence Matrix

### Core Functions (All Three — 77 functions)

All three keyboards share these foundational functions:

**System Init/Management:**
`Board_Wakeup_Init`, `Init_Batt_Infomation`, `Init_Gpio_Infomation`, `Init_Keyboard_Infomation`, `User_Keyboard_Init`, `User_Keyboard_Post_Init`, `User_Keyboard_Reset`, `User_Systime_Init`, `User_Systime_Deinit`, `Save_Flash_Set`, `es_mcu_reset`, `mcu_reset`, `bootloader_jump`

**Wireless/SPI:**
`Spi_Main_Loop`, `Spi_Send_Commad`, `Spi_Ack_Send_Commad`, `Get_Spi_Return_Data`, `es_ble_spi_init`, `es_ble_spi_deinit`, `es_spi_send_recv_by_dma`, `Ble_Name_Synchronization`, `Mode_Synchronization`, `User_bluetooth_send_keyboard`

**Battery/ADC:**
`User_Adc_Init`, `User_Adc_Deinit`, `User_Adc_Batt_Number`, `User_Adc_Batt_Power_Up_Init`, `Init_Batt_Infomation`, `U16_Buff_Clear`

**EEPROM/Flash:**
`eeprom_driver_init`, `eeprom_driver_erase`, `eeprom_write_block`, `eeprom_read_block`, `eeprom_write_block_user`, `eeprom_read_block_user`

**RGB Matrix:**
`rgb_matrix_driver_init`, `rgb_matrix_driver_flush`, `rgb_matrix_driver_flush_pwm_dma_start`, `rgb_matrix_driver_set_color`, `rgb_matrix_driver_set_color_all`, `User_Pwm_Deinit`

**LED Effects:**
`User_Led_Show`, `Led_Rf_Mode_Show`, `Led_Batt_Number_Show`, `Led_Point_Flash_Show`, `Led_Power_Low_Show`

**USB:**
`User_Usb_Init`, `User_Usb_Deinit`, `es_restart_usb_driver`, `Usb_Disconnect`

**Power:**
`User_Sleep`, `User_Wakeup`, `es_chibios_user_idle_loop_hook`

**HID:**
`es_keyboard_leds`, `es_send_keyboard`, `es_send_nkro`, `es_send_mouse`, `es_send_extra`, `es_change_qmk_nkro_mode_enable`, `es_change_qmk_nkro_mode_disable`

**EMI:**
`Emi_Init`, `Emi_Read_Data`, `Emi_Write_Data`

**Data Queues:**
`app_2g4_buffer_full`, `app_2g4_buffer_empty`, `app_2g4_buffer_rev_add`, `app_2g4_buffer_send_add`

**Misc:**
`clamp_length`, `clamp_length_user`, `User_Get_Led_Power_Status`

**Interrupt Vectors:**
`Vector4C`, `Vector54`, `Vector58`, `Vector5C`, `Vector68`, `Vector78`, `Vector7C`, `Vector80`, `Vector84`

### RT82-Only Functions (34 functions)

These implement the **LCD display** and **mode switch** features:

| Function | Category |
|---|---|
| `Lvgl_Init` | Display initialization |
| `Lvgl_Boot` | Display boot sequence |
| `Reset_Lvgl_Boot` | Display reset boot |
| `Reset_Power_Lvgl` | Display power reset |
| `App_Uart_Main_Loop` | UART main loop for LCD |
| `Init_Uart_Information` | UART init for LCD |
| `Uart_Dma_Init` | UART DMA setup |
| `Uart_Tx_Dma_Start` | UART TX DMA |
| `Uart_Rx_Dma_Start` | UART RX DMA |
| `User_Uart_Send_Data` | UART data send |
| `Uart_While_Send_Commad` | UART blocking send |
| `Reset_Uart` | UART reset |
| `User_Uart_Deinit` | UART deinit |
| `Scan_Uart_Rf` | LCD RF status update |
| `Scan_Uart_Work_Mode` | LCD work mode update |
| `Scan_Uart_Func` | LCD function update |
| `Scan_Uart_Batt` | LCD battery update |
| `Scan_Uart_Gif` | LCD GIF update |
| `Get_Lcd_Usb_Work_Status` | LCD USB status check |
| `Lcd_Usb_Work_Init` | LCD USB init |
| `Scan_Lcd_Usb_Work_Mode` | LCD USB mode scan |
| `Uart_Key_Mode_Key_Scan` | LCD key event handler |
| `Lcd_Work_Sync_GIF` | GIF sync |
| `Key_Switch_Mode_Scan` | Physical mode switch |
| `Key_Switch_Mode_Power` | Mode switch power |
| `Sleep_Time_Synchronization` | Sleep time config |
| `DSleep_Time_Synchronization` | Deep sleep config |
| `Send_Key` | Key send helper |
| `User_Clear_Board_2ms` | Board clear with delay |
| `User_Consumer_Send` | Consumer key send |
| `User_process_record_user` | Custom keycode handler |
| `Vector64` | UART TX DMA IRQ |
| `Vector6C` | UART RX DMA IRQ |
| `add_key_to_report` | (also in Evo80) |

### RD75-Only Functions (3 functions)

| Function | Category |
|---|---|
| `encoder_driver_init` | Rotary encoder init |
| `encoder_quadrature_handle_read` | Encoder read |
| `encoder_quadrature_post_init` | Encoder post-init |

### Evo80-Only Functions (12 functions)

| Function | Category |
|---|---|
| `Key_Value_Dispose` | Key value processing |
| `Logo_Breath_Mix_mode_Show` | Logo mixed breath |
| `Logo_Light_mode_Show_Blue` | Logo blue light |
| `Logo_Light_mode_Show_Green` | Logo green light |
| `Logo_Light_mode_Show_Pink` | Logo pink light |
| `Logo_Light_mode_Show_White` | Logo white light |
| `Logo_Light_mode_Show_Yellow` | Logo yellow light |
| `Logo_Wave_Mix_mode_Show` | Logo mixed wave |
| `Logo_Wave_Rgb_1_mode_Show` | Logo RGB wave 1 |
| `Logo_Wave_Rgb_2_mode_Show` | Logo RGB wave 2 |
| `Usb_Suspend_Show` | USB suspend display |
| `User_Mac_Win_Change` | Mac/Win mode switch |

### RD75 + Evo80 Shared (not RT82) — 14 functions

These implement **Logo LED effects** (both have `LOGO_LED_ENABLE=1`):

`Logo_Init`, `Logo_Mode_Show`, `Logo_Breath_mode_Show`, `Logo_Light_mode_Show`, `Logo_Off_mode_Show`, `Logo_Pwm_Ds_Updata`, `Logo_Pwm_Rgb_Updata`, `Logo_Spectrum_mode_Show`, `Logo_Wave_Ds_mode_Show`, `Logo_Wave_Rgb_mode_Show`, `User_Point_Show`, `User_Via_Qmk_Logo_Command`, `User_Via_Qmk_Logo_Get_Value`, `User_Via_Qmk_Logo_Set_Value`

### RT82 + Evo80 Shared (not RD75) — 6 functions

Custom key report handling: `General_Key_Reorder`, `User_Clear_Board`, `User_Get_Led_Power_Status`, `User_send_6kro_report`, `User_send_nkro_report`, `del_key_from_report`

### RT82 + RD75 Shared (not Evo80) — 1 function

`User_Test_Colour_Show` (LED test mode)

## External Dependency Comparison

| Dependency | RT82 | RD75 | Evo80 |
|---|---|---|---|
| `md_uart_init` | Yes | No | No |
| `md_dma_set_channel_data_start_address_and_length` | Yes | No | No |
| `timer_read` / `timer_elapsed` | Yes | No | Yes |
| `host_keyboard_send` / `host_nkro_send` / `host_consumer_send` | Yes | No | Yes |
| `host_keyboard_led_state` | Yes | No | Yes |
| `keyboard_protocol` / `keyboard_report` / `nkro_report` | Yes | No | Yes |
| `real_mods` / `weak_mods` / `clear_mods` / `clear_weak_mods` | Yes | No | Yes |
| `rgb_matrix_get_hue/sat/speed` | Yes | No | Yes |
| `rgb_matrix_sethsv_noeeprom` | Yes | No | Yes |
| `encoder_queue_event` | No | Yes | No |
| `layer_move` | No | No | Yes |
| `__gnu_thumb1_case_uhi` | Yes | No | Yes |
| `__gnu_thumb1_case_shi` | No | No | Yes |

## Key Insights

1. **Shared codebase**: 77 functions are common to all three, confirming they share a single proprietary codebase with conditional compilation.

2. **RT82 is the largest**: At 17,932 bytes of code, RT82 is ~43% larger than RD75 (12,512 bytes), primarily due to the LCD/UART display subsystem.

3. **Feature gating via `#define`**: The `LOGO_LED_ENABLE` and `SIDE_LED_ENABLE` macros gate entire feature sets. The RT82 uniquely uses a UART-connected LCD instead of logo LEDs.

4. **Encoder only in RD75**: The RD75 includes a hardware rotary encoder driver. The RT82 handles its encoder in the QMK layer instead.

5. **Report handling varies**: RT82 and Evo80 implement custom 6KRO/NKRO report building with `add_key_to_report`/`del_key_from_report`/`General_Key_Reorder`. RD75 uses simpler key handling.

6. **Same MCU family**: All three target the same EastSoft MCU (FS026), use the same flash controller API (`md_fc_*`), same USB API (`ald_usb_*`), and same SPI wireless module protocol.

7. **Deep sleep is RT82-only**: Two-stage sleep (`Sleep_Time_Synchronization` + `DSleep_Time_Synchronization`) only appears in the RT82, likely for battery optimization with the power-hungry LCD.

8. **Evo80 has the most Logo effects**: 10+ unique Logo LED functions, suggesting it has a more sophisticated underglow/logo lighting system.
