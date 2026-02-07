# Reverse Engineering — librdrcommon.a

## Purpose

This document tracks the reverse engineering effort of the proprietary `librdrcommon.a` binary library included in the Epomaker RT82 QMK firmware. The goal is to understand the binary's functionality sufficiently to create an open-source reimplementation, restoring GPL compliance.

See also: [BINARY_LIBRARY_NOTICE.md](BINARY_LIBRARY_NOTICE.md) for the GPL violation details.

## Documentation Index

| Document | Description |
|---|---|
| [docs/binary_analysis/function_signatures.md](docs/binary_analysis/function_signatures.md) | Complete function catalog with signatures, sizes, and categories |
| [docs/binary_analysis/comparison_matrix.md](docs/binary_analysis/comparison_matrix.md) | Side-by-side comparison of RT82, RD75, and Evo80 binaries |
| [docs/binary_analysis/protocol_notes.md](docs/binary_analysis/protocol_notes.md) | Communication protocols (SPI wireless, UART LCD, WS2812, EEPROM, ADC) |

### Raw Analysis Data

| Directory | Contents |
|---|---|
| `docs/binary_analysis/rt82/` | RT82 symbols, disassembly, strings, sections, ELF symbols |
| `docs/binary_analysis/rd75/` | RD75 symbols, disassembly, strings, sections, ELF symbols |
| `docs/binary_analysis/evo80/` | Evo80 symbols, disassembly, strings, sections, ELF symbols |

## Summary of Findings

### Binary Profile

- **Single object file**: `rdr_common.o` compiled from one large C source file
- **Architecture**: ARM Cortex-M0 (Thumb-1 ISA), ELF32, little-endian, EABI v5
- **Compiler**: GCC 10.1.0 (arm-none-eabi), built on Windows via QMK MSYS
- **MCU**: EastSoft FS026 (custom ARM Cortex-M0)
- **Code size**: 17,932 bytes (.text), 313 bytes (.data), 3,577 bytes (.bss)

### Function Breakdown

| Category | Count | Key Functions |
|---|---|---|
| LCD/Display (RT82-only) | 23 | `Lvgl_Init`, `App_Uart_Main_Loop`, `Scan_Uart_*` |
| Wireless/SPI | 10 | `Spi_Main_Loop`, `es_ble_spi_init`, `User_bluetooth_send_keyboard` |
| Battery/ADC | 6 | `User_Adc_Batt_Number`, `User_Adc_Init` |
| EEPROM/Flash | 8 | `eeprom_driver_init`, `eeprom_write_block_user` |
| RGB Matrix | 6 | `rgb_matrix_driver_*` |
| LED Effects | 7 | `User_Led_Show`, `Led_Rf_Mode_Show` |
| System Init | 11 | `User_Keyboard_Init`, `Init_Gpio_Infomation` |
| Sleep/Wake | 6 | `es_chibios_user_idle_loop_hook` (1,484 bytes — largest function) |
| USB | 4 | `User_Usb_Init`, `es_restart_usb_driver` |
| Key Reports/HID | 13 | `es_send_keyboard`, `User_process_record_user` (1,460 bytes) |
| Interrupts | 9-11 | `Vector78` (1,400 bytes — main timer ISR) |
| EMI Test | 3 | `Emi_Init`, `Emi_Read_Data` |
| Data Queues | 4 | `app_2g4_buffer_*` |
| Mode Switch (RT82-only) | 2 | `Key_Switch_Mode_Scan` |

**Total**: 118 functions (RT82), 95 (RD75), 109 (Evo80). **77 functions shared by all three.**

### Three Largest Functions

1. **`es_chibios_user_idle_loop_hook`** (1,484 bytes) — Main system tick loop. Handles mode switching, sleep timers, LED updates, SPI polling, battery monitoring. This is the central coordination point.

2. **`User_process_record_user`** (1,460 bytes) — Custom keycode handler. Processes all custom keycodes (mode switch, battery display, LVGL control, debounce settings, etc.). RT82-only.

3. **`Vector78`** (1,400 bytes) — Main timer/SysTick interrupt handler. Manages timing for LED effects, SPI communication intervals, sleep counters, and UART polling.

### External Dependencies

The binary links against 60 external symbols from:
- **QMK core** (29): keyboard reports, host driver, RGB matrix, keymaps, timers
- **ChibiOS** (4): USB driver, thread sleep, IRQ handling, GPIO
- **EastSoft MCU SDK** (17): GPIO, SPI, UART, ADC, flash controller, USB
- **C runtime** (8): memcpy/memset/memcmp, integer division, switch table helpers

## Reimplementation Priority

### Phase 1 — Easiest to Reimplement

These functions have well-understood behavior or open-source equivalents:

1. **EEPROM driver** — Standard flash-emulated EEPROM pattern. The `md_fc_*` API is documented in EastSoft SDK.
2. **RGB Matrix driver** — WS2812 PWM+DMA, well-known protocol. Timing values are in header constants.
3. **Data queue** (`app_2g4_buffer_*`) — Simple ring buffer, trivial to reimplement.
4. **Battery ADC** — Standard ADC read with averaging and voltage thresholds.
5. **GPIO init** — Pin configuration from header `#define` values.

### Phase 2 — Medium Complexity

6. **Key report building** — Custom 6KRO/NKRO report assembly. Need to match exact report format.
7. **LED effects** — State machine for mode indicators, battery display, pairing animation.
8. **USB management** — USB connect/disconnect/restart using EastSoft `ald_usb_*` API.
9. **Sleep/wake** — Power management with GPIO wakeup configuration.

### Phase 3 — Hardest / Most Critical

10. **SPI wireless protocol** — The core wireless communication. 64-byte frame format, command set documented. Need to reverse-engineer timing and handshake details from `Spi_Main_Loop` and `Get_Spi_Return_Data`.
11. **`es_chibios_user_idle_loop_hook`** — Central coordination loop. Must replicate exact timing and state machine behavior.
12. **`User_process_record_user`** — Full custom keycode handler with mode switching, display control, etc.

### Phase 4 — RT82-Specific

13. **UART LCD protocol** — Display communication. Well-documented via [rt82display](https://github.com/guysoft/rt82display) project. The UART command format is clear from the header enums.

## Tools Used

- `arm-none-eabi-nm` — Symbol extraction
- `arm-none-eabi-objdump` — Disassembly and symbol tables
- `arm-none-eabi-readelf` — ELF structure analysis
- `arm-none-eabi-size` — Section size analysis
- `strings` — String extraction

## Contributing

To contribute to the reimplementation effort:

1. Focus on Phase 1 functions first — they're self-contained and testable
2. Use the RT82 display protocol docs as reference for UART functions
3. Compare behavior across all three keyboard binaries for validation
4. Test replacements against the binary firmware to verify behavior matches

## Legal Note

This reverse engineering is conducted for **interoperability** purposes under applicable laws. The goal is to create a clean-room reimplementation of the functionality described by the public header file `rdr_common.h`, not to copy the proprietary code.
