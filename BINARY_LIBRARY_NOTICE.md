# Binary Library Notice — Epomaker RT82

## Overview

This repository contains a **precompiled binary library** (`lib/rdr_lib/librdrcommon.a`) for which **no source code has been provided**. This library is linked into the firmware and provides critical functionality that cannot be independently built or audited.

The rest of the QMK firmware in this repository is licensed under the **GNU General Public License, version 2 or later** (GPL-2.0-or-later), as stated in source file headers. Including a binary-only library without providing corresponding source code violates the terms of the GPL.

## Binary Library Details

| Property | Value |
|---|---|
| **File** | `lib/rdr_lib/librdrcommon.a` |
| **Size** | 282,828 bytes (276 KB) |
| **SHA-256** | `3fab0cf31ac71919bf0651d9c9c98ac9e26e735bcaeac922e406615ddb0fe553` |
| **Format** | ELF32 ARM relocatable object archive (single object: `rdr_common.o`) |
| **Architecture** | ARM (EABI version 5, little-endian) |
| **Sections** | 408 ELF sections |
| **Compiled with** | GCC 10.1.0 (arm-none-eabi) |
| **Build host path** | `c:\qmk_msys\mingw64\lib\gcc\arm-none-eabi\10.1.0\include` |
| **Header file** | `lib/rdr_lib/rdr_common.h` (35,660 bytes — API declarations only) |

## Functionality Provided by the Binary

Based on the header file (`rdr_common.h`), the library implements:

### Wireless Communication
- SPI-based 2.4 GHz wireless protocol (`Spi_Main_Loop`, `es_ble_spi_init`)
- Bluetooth LE support for 3 channels (`USER_SWITCH_BLE_1_MODE` through `USER_SWITCH_BLE_3_MODE`)
- USB/wireless mode switching (`Mode_Synchronization`)
- Custom host driver implementation (`es_user_driver`)

### Display / LCD
- LVGL display initialization and control (`Lvgl_Init`, `Reset_Power_Lvgl`)
- UART-based LCD communication protocol (multiple `Command_*` enums)
- GIF animation support on 240x135 RGB565 LCD
- Home/Function/Enter screen layer navigation

### Battery Management
- ADC-based battery level reading (`User_Adc_Batt_Number`)
- Charge detection via `ES_BATT_STDBY_IO`
- Low power shutdown thresholds

### Storage
- Custom EEPROM driver (`eeprom_write_block_user`, `eeprom_read_block_user`, `eeprom_driver_init`)
- Flash save/restore functionality

### RGB LED Control
- WS2812 PWM+DMA driver for 82 LEDs (`rgb_matrix_driver_flush_pwm_dma_start`)
- Full `rgb_matrix_driver_t` implementation

### Keyboard Core
- Matrix scanning with custom GPIO pin assignments
- 6-key rollover and N-key rollover switching
- Debounce management
- Sleep/wake power management
- Mode switch (physical toggle) scanning

## GPL Compliance Issue

### The Problem

QMK Firmware is distributed under the **GNU General Public License, version 2 or later**. Section 3 of GPL-2.0 requires that when distributing a binary derived from GPL-licensed source code, the **complete corresponding source code** must also be made available.

The file `lib/rdr_lib/librdrcommon.a` is a compiled binary object that is **linked directly into the GPL-licensed firmware** at build time. No source code for this library has been provided. This means:

1. Users **cannot inspect** what the wireless and display code does
2. Users **cannot modify** wireless behavior, battery management, or display features
3. Users **cannot rebuild** the complete firmware from source
4. Users **cannot verify** the code is free of security issues or unwanted behavior

### Scope of Impact

This is not a minor utility — the binary library implements the **majority of the keyboard's hardware-specific functionality**. Without its source code, the "open source" firmware release is incomplete.

### Similar Keyboards Affected

The same pattern appears in firmware releases for other keyboards:

| Keyboard | Binary Size | SHA-256 |
|---|---|---|
| **Epomaker RT82** | 282,828 bytes | `3fab0cf3...` |
| **Epomaker Evo80** | 257,694 bytes | `89ae699c...` |
| **Womier RD75** | 205,504 bytes | `0d6e2cf7...` |

All three use the same `librdrcommon.a` / `rdr_common.h` API pattern, suggesting a shared proprietary codebase.

## How to Request Source Code

Under GPL-2.0 Section 3, you have the right to request the complete source code. Contact:

- **Epomaker**: https://github.com/Epomaker
- **Epomaker Support**: https://epomaker.com/pages/contact-us
- **QMK Community**: https://github.com/qmk/qmk_firmware

When requesting, reference:
- The GPL-2.0-or-later license headers in the source files
- The specific binary file `lib/rdr_lib/librdrcommon.a`
- The corresponding header `lib/rdr_lib/rdr_common.h` showing the API surface

## Building the Firmware

Despite the GPL compliance issue, the firmware **can be built** using the precompiled binary:

```bash
qmk compile -kb epomaker/epomaker_rt82 -km default
```

This produces `epomaker_epomaker_rt82_default.bin` (78,080 bytes). The build links against the proprietary `librdrcommon.a` to produce a functional firmware image.

**Note**: By building and using this firmware, you accept that a significant portion of the code running on your keyboard cannot be inspected or modified.

## Reverse Engineering Status

Work is underway to analyze the binary library and document its behavior, with the long-term goal of creating an open-source reimplementation. See `docs/binary_analysis/` for ongoing analysis documentation.

---

*This notice was created on 2026-02-07 to document the GPL compliance issue for community awareness.*
