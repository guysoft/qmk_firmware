# Communication Protocols — librdrcommon.a

## 1. SPI Wireless Protocol (All Keyboards)

The wireless communication (2.4GHz and BLE) uses SPI2 with DMA to talk to an external RF module.

### Hardware Configuration

- **SPI peripheral**: SPI2 (configured via `SPI2_InitStruct`)
- **DMA channels**: Channel 2 primary (`g_es_dma_ch2pri_cfg`) and alternate (`g_es_dma_ch2alt_cfg`)
- **Buffer size**: 64 bytes TX (`g_es_spi_tx_buf`), 64 bytes RX (`g_es_spi_rx_buf`)
- **ACK pin**: `ES_SPI_ACK_IO` (A4) — RF module acknowledge
- **Mode pins**: `MODE_2P4G_IO` (B13), `MODE_BLE_IO` (B12)

### SPI Command Protocol

Commands are sent as the first byte of a 64-byte SPI frame:

| Command | Value | Direction | Description |
|---|---|---|---|
| `USER_SWITCH_2P4G_MODE` | `0x00` | TX | Switch to 2.4GHz mode |
| `USER_SWITCH_BLE_1_MODE` | `0x01` | TX | Switch to BLE channel 1 |
| `USER_SWITCH_BLE_2_MODE` | `0x02` | TX | Switch to BLE channel 2 |
| `USER_SWITCH_BLE_3_MODE` | `0x03` | TX | Switch to BLE channel 3 |
| `USER_SWITCH_2P4G_PAIR` | `0x04` | TX | Enter 2.4GHz pairing |
| `USER_SWITCH_BLE_1_PAIR` | `0x05` | TX | Enter BLE 1 pairing |
| `USER_SWITCH_BLE_2_PAIR` | `0x06` | TX | Enter BLE 2 pairing |
| `USER_SWITCH_BLE_3_PAIR` | `0x07` | TX | Enter BLE 3 pairing |
| `USER_SWITCH_USB_MODE` | `0x08` | TX | Switch to USB mode |
| `USER_KEYBOARD_SLEEP` | `0x09` | TX | Enter sleep mode |
| `USER_KEYBOARD_WAKEUP` | `0x0A` | TX | Wake from sleep |
| `USER_KEY_BYTE_DATA` | `0x0B` | TX | 6KRO key report (8 bytes) |
| `USER_KEY_BIT_DATA` | `0x0C` | TX | NKRO key report (15 bytes) |
| `USER_MOUSE_DATA` | `0x0D` | TX | Mouse report (8 bytes) |
| `USER_CONSUMER_DATA` | `0x0E` | TX | Consumer key report (3 bytes) |
| `USER_SYSTEM_DATA` | `0x0F` | TX | System key report (3 bytes) |
| `USER_BATTERY_DATA` | `0x10` | TX | Battery level (2 bytes) |
| `USER_GET_RF_STATUS` | `0x11` | TX/RX | Query RF connection status |
| `USER_BLE1_WRITE_NAME` | `0x12` | TX | Set BLE channel 1 name |
| `USER_BLE2_WRITE_NAME` | `0x13` | TX | Set BLE channel 2 name |
| `USER_BLE3_WRITE_NAME` | `0x14` | TX | Set BLE channel 3 name |
| `USER_SLEEP_TIME_WRITE` | `0x15` | TX | Set stage-1 sleep timeout |
| `USER_DSLEEP_TIME_WRITE` | `0x16` | TX | Set stage-2 deep sleep timeout |
| `USER_EMI_COMMAND` | `0xBB` | TX/RX | EMI test mode command |
| `USER_KEYBOARD_COMMAND` | `0x0A` | TX | Keyboard command (64-byte frame) |

### Report Format

Reports are encapsulated in the SPI frame with report ID + data:

| Report | ID | Length | Content |
|---|---|---|---|
| 6KRO Keyboard | `KB_REPORT_ID` (0x06) | 8 bytes | Standard USB HID keyboard report |
| NKRO Keyboard | `KB_REPORT_ID` (0x06) | 15 bytes | Bitmap keyboard report |
| Mouse | `MOUSE_REPORT_ID` (0x02) | 8 bytes | Standard USB HID mouse report |
| Consumer | `CON_REPORT_ID` (0x04) | 3 bytes | Consumer control report |
| System | `SYS_REPORT_ID` (0x03) | 3 bytes | System control report |

### SPI Timing

- RF mode interval: `SPI_DELAY_RF_TIME` = 100 (ticks/ms)
- USB mode interval: `SPI_DELAY_USB_TIME` = 1500 (ticks/ms)
- ACK/NACK polling before send
- Busy/Idle state tracking via `Spi_Send_Recv_Flg`

### Connection Status

The RF module returns status via `USER_GET_RF_STATUS`:

- `BLE_24G_NONE` (0) — No connection
- `BLE_24G_PIAR` (1) — Pairing in progress
- `BLE_24G_RETURN` (2) — Reconnecting

System tracks: `KB_MODE_CONNECT_OK`, `KB_MODE_CONNECT_PAIR`, `KB_MODE_CONNECT_RETURN`

## 2. UART LCD Protocol (RT82 Only)

The RT82's LCD screen communicates via UART with DMA. This is a dedicated serial link to a separate display controller.

### Hardware Configuration

- **UART TX buffer**: 8 bytes (`Uart_Tx_Buffer[UART_BUF_SIZE]`)
- **UART RX buffer**: 8 bytes (`Uart_Rx_Buffer[UART_BUF_SIZE]`)
- **DMA**: Channel 0 TX (`g_es_dma_ch0_cfg`), Channel 1 RX (`g_es_dma_ch1_cfg`)
- **Power pin**: `LVGL_POWER_IO` (A8)
- **Reset pin**: `LVGL_RESET_IO` (A5)
- **Boot pin**: `LVGL_BOOT_IO` (A6)

### UART Command Format

Commands are 4-byte packets with optional data:

| Command | Value | Direction | Description |
|---|---|---|---|
| `Command_Get_Error` | `0x55` | RX | Error from display |
| `Command_Get_Status` | `0xAA` | RX | Status from display |
| `Command_Ready_Ok` | `0xBB` | RX | Display ready acknowledgement |
| `Command_Work_Mode_Data` | `0xC0` | TX | Set display work mode (Home/Key/Gif/Test) |
| `Command_Led_Data` | `0xC1` | TX | Send LED status to display |
| `Command_Batt_Data` | `0xC2` | TX | Send battery level to display |
| `Command_Mode_Data` | `0xC3` | TX | Send connection mode to display |
| `Command_Key_Mode_Data` | `0xC4` | TX | Send key mode info to display |
| `Command_Home_Mov_Data` | `0xC5` | TX | Send home screen navigation |
| `Command_Gif_Change_Data` | `0xC6` | TX | Change GIF animation |
| `Command_Test_Colour` | `0xC7` | TX | Test colour display |
| `Command_Usb_Data` | `0xC8` | TX | Send USB status to display |
| `Command_USB_DISABLE` | `0xDD` | TX | Disable USB on display |

### UART Sub-Commands (from header defines)

| Sub-Command | Value | Description |
|---|---|---|
| `USER_UART_COMMAND1` | `0x11` | UART packet type 1 |
| `USER_UART_COMMAND2` | `0x22` | UART packet type 2 |
| `USER_UART_COMMAND3` | `0x33` | UART packet type 3 |
| `USER_UART_COMMAND4` | `0x44` | UART packet type 4 |

### Display Status Indicators (bitmask)

| Bit | Mask | Meaning |
|---|---|---|
| Num Lock | `0x001` | Num Lock LED state |
| Caps Lock | `0x002` | Caps Lock LED state |
| Scroll Lock | `0x004` | Scroll Lock LED state |
| Win Lock | `0x008` | Windows key locked |
| Mac/Win | `0x010` | Mac mode (vs Windows) |

### Display Modes

| Mode | Enum | Description |
|---|---|---|
| Home | `Uart_Home` (0) | Default home screen |
| Key | `Uart_Key` (1) | Key visualization mode |
| GIF | `Uart_Gif` (2) | GIF animation mode |
| Test | `Uart_Test` (3) | Test/debug mode |

### Navigation Layers

The display has a 3-layer navigation system:

1. **Home Layer**: Default view, Volume Up/Down indicators
2. **Function Layer**: Mode change, Mac/Win selection
3. **Enter Layer**: USB/2.4G/BLE1/BLE2/BLE3/Win/Mac confirmations

GIF animations: 3 slots (`GIF_PIC_MAX = 3`), with `GIF_MODE_1`, `GIF_MODE_2`, `GIF_MODE_3`.

### LCD Power Sequence

1. Power on: `LVGL_POWER_IO` (A8) high
2. Wait for `Command_Ready_Ok` (0xBB) from display
3. Send initial mode via `Command_Work_Mode_Data`
4. 200ms delay after power up (`Uart_Lvgl_200ms_Delay`)

### Related Project

The UART protocol is documented independently at [guysoft/rt82display](https://github.com/guysoft/rt82display).

## 3. WS2812 RGB LED Protocol (All Keyboards)

### Hardware

- **PWM output**: `ES_PWM_DMA_IO` (A2)
- **LED count**: 82 (RT82) — varies per keyboard
- **Protocol**: WS2812 one-wire, bit-banged via PWM timer + DMA
- **SDB power**: `ES_SDB_POWER_IO` (A3) — LED driver enable
- **LED power**: `ES_LED_POWER_IO` (D0) — LED power rail

### Timing

- High bit: `ES_PWM_WS2812_H_VALUE` = 43 timer ticks
- Low bit: `ES_PWM_WS2812_L_VALUE` = 17 timer ticks
- 24 bits per LED (GRB order, standard WS2812)
- DMA transfers entire LED chain in one shot

### Buffer Layout

```
g_es_pwm_rgb_matrix_array_dma_buf[LED_COUNT * 24 + 2]
```

Each LED = 24 bytes in DMA buffer (one byte per bit, holding PWM duty cycle value).

## 4. EEPROM Emulation Protocol

### Implementation

Flash-based EEPROM emulation with wear-leveling:
- `g_es_flash_eeprom_table` — Page table for wear-leveling
- `IAPROM_PAGE_ERASE` — In-Application Programming erase
- `IAPROM_WORD_PROGRAM` — In-Application Programming write
- `ee_format` — Format/initialize EEPROM pages
- `ee_verify_pagefull_write_variable` — Write with page full check

### API

Standard QMK EEPROM API redirected through custom driver:
- `eeprom_driver_init()` — Initialize flash pages, check/format
- `eeprom_write_block_user(buf, addr, len)` — Write with wear-leveling
- `eeprom_read_block_user(buf, addr, len)` — Read from current page
- `eeprom_driver_erase()` — Full erase

Uses EastSoft flash controller:
- `md_fc_unlock()` → `md_fc_page_erase()` / `md_fc_program()` → `md_fc_lock()`

## 5. ADC Battery Monitoring

### Configuration

- ADC init struct: `adc_initStruct` (read-only data)
- Calibration: `md_adc_calibration()` called during init
- Sample count: 10 samples (`USER_BATT_SCAN_COUNT`)
- Power-up scan count: 10 (`USER_BATT_POWER_SCAN_COUNT`)

### Voltage Thresholds

| Level | ADC Value | Voltage | Action |
|---|---|---|---|
| Full | 2555 | ~4.13V | 100% battery |
| Low | 2090 | ~3.32V | Low battery warning |
| Shutdown | 1865 | ~3.04V | Force shutdown |

Formula: `V = (ADC * 3.3 / 4096) * 2` (voltage divider factor 2)

### Reporting

- Battery percentage sent to SPI module via `USER_BATTERY_DATA`
- Battery status sent to LCD via `Command_Batt_Data` (RT82 only)
- 15-second scan interval (`User_Batt_Time_15S_Count`)
- Power-up delay: 25 seconds (`USER_BATT_DELAY_TIME`)
