#!/usr/bin/env python3
"""
WS2812 PWM+DMA diagnostic tool for Epomaker RT82.

Communicates with the keyboard over RAW HID (VIA debug command 0xFE)
to test individual LEDs, read ISR timing stats, and dump frame buffer
contents -- all without reflashing.

Prerequisites:
    pip install hidapi

Usage:
    ws2812_diag.py set <led> <r> <g> <b>              Set one LED
    ws2812_diag.py range <start> <count> <r> <g> <b>  Set a range
    ws2812_diag.py all <r> <g> <b>                     Set all LEDs
    ws2812_diag.py timing                              Read ISR timing stats
    ws2812_diag.py dump <offset> [count]               Dump frame buffer (default count=14)
    ws2812_diag.py freeze [0|1]                        Freeze/unfreeze RGB matrix
    ws2812_diag.py boundary-test                       Automated boundary test
    ws2812_diag.py sweep                               Light LEDs one-by-one to find corruption
"""

import sys
import time
import struct

try:
    import hid
except ImportError:
    print("ERROR: 'hid' or 'hidapi' package required.")
    print("  Install with: pip install hid    (or: pip install hidapi)")
    sys.exit(1)

# Detect which hid package variant is installed:
#   - 'hid' package:   hid.Device, dev.write(), dev.read()
#   - 'hidapi' package: hid.device, dev.open_path(), dev.write(), dev.read()
_HAS_DEVICE_CLASS = hasattr(hid, 'Device')  # 'hid' package (uppercase)

# ── Keyboard identifiers ──────────────────────────────────────────────
VID = 0x36B0
PID = 0x30A3
RAW_USAGE_PAGE = 0xFF60
RAW_USAGE_ID   = 0x61

# ── Protocol constants ────────────────────────────────────────────────
CMD         = 0xFE
SUB_SET_LED     = 0x01
SUB_SET_RANGE   = 0x02
SUB_SET_ALL     = 0x03
SUB_GET_TIMING  = 0x04
SUB_DUMP_BUFFER = 0x05
SUB_FREEZE_RGB  = 0x06

REPORT_LEN = 32  # RAW HID report size

# ── LED layout info ───────────────────────────────────────────────────
LED_COUNT = 82
DMA_BATCH_1_BOUNDARY = 1024  # bit index where batch 2 starts
BITS_PER_LED = 24

LED_NAMES = {
    0: "Esc", 1: "F1", 2: "F2", 3: "F3", 4: "F4", 5: "F5", 6: "F6",
    7: "F7", 8: "F8", 9: "F9", 10: "F10", 11: "F11", 12: "F12", 13: "PrtSc",
    14: "~", 15: "1", 16: "2", 17: "3", 18: "4", 19: "5", 20: "6",
    21: "7", 22: "8", 23: "9", 24: "0", 25: "-", 26: "=", 27: "Bksp", 28: "Home",
    29: "Tab", 30: "Q", 31: "W", 32: "E", 33: "R", 34: "T", 35: "Y",
    36: "U", 37: "I", 38: "O", 39: "P", 40: "[", 41: "]", 42: "\\", 43: "Del",
    44: "Caps", 45: "A", 46: "S", 47: "D", 48: "F", 49: "G", 50: "H",
    51: "J", 52: "K", 53: "L", 54: ";", 55: "'", 56: "Enter", 57: "PgDn",
    58: "LShift", 59: "Z", 60: "X", 61: "C", 62: "V", 63: "B", 64: "N",
    65: "M", 66: ",", 67: ".", 68: "/", 69: "RShift", 70: "Up", 71: "End",
    72: "LCtrl", 73: "LWin", 74: "LAlt", 75: "Space", 76: "RAlt", 77: "Fn",
    78: "Left", 79: "Down", 80: "Right", 81: "Ins",
}


def led_info(idx):
    """Return a string like '42 (\\) bits 1008-1031 batch 1/2'."""
    name = LED_NAMES.get(idx, "?")
    bit_start = idx * BITS_PER_LED
    bit_end = bit_start + BITS_PER_LED - 1
    batch = 1 if bit_start < DMA_BATCH_1_BOUNDARY else 2
    if bit_start < DMA_BATCH_1_BOUNDARY <= bit_end:
        batch_str = "straddles 1/2"
    else:
        batch_str = f"batch {batch}"
    return f"LED {idx:2d} ({name:>6s}) bits {bit_start:4d}-{bit_end:4d} {batch_str}"


class RT82Debug:
    def __init__(self):
        self.dev = None

    def open(self):
        """Open the RAW HID interface of the RT82."""
        devices = hid.enumerate(VID, PID)

        target_path = None

        # First try matching by usage page (works on macOS/Windows)
        for info in devices:
            if info["usage_page"] == RAW_USAGE_PAGE and info["usage"] == RAW_USAGE_ID:
                target_path = info["path"]
                break

        # Linux hidraw backend often reports usage_page=0; fall back to
        # interface number.  On the ES32/RT82 the RAW HID interface is 1
        # (0=keyboard, 1=RAW HID, 2=extra/NKRO).
        if target_path is None:
            RAW_HID_IFACE = 1
            for info in devices:
                if info["interface_number"] == RAW_HID_IFACE:
                    target_path = info["path"]
                    break

        if target_path is None:
            raise RuntimeError(
                f"RT82 RAW HID interface not found (VID={VID:#06x} PID={PID:#06x}).  "
                f"Is the keyboard plugged in?")

        self.dev = hid.device()
        self.dev.open_path(target_path)
        print(f"Connected: path={target_path.decode()}")

    def close(self):
        if self.dev:
            self.dev.close()

    def _send(self, data):
        """Send a 32-byte report and read the 32-byte response."""
        buf = bytearray(REPORT_LEN + 1)  # +1 for report ID prefix (0x00)
        buf[0] = 0x00  # report ID
        buf[1:1 + len(data)] = data
        self.dev.write(bytes(buf))
        resp = self.dev.read(REPORT_LEN, timeout_ms=2000)
        if not resp:
            raise TimeoutError("No response from keyboard (2s timeout)")
        return bytes(resp)

    # ── Commands ──────────────────────────────────────────────────────

    def set_led(self, idx, r, g, b):
        resp = self._send(bytes([CMD, SUB_SET_LED, idx, r, g, b]))
        return resp[2] == 0

    def set_range(self, start, count, r, g, b):
        resp = self._send(bytes([CMD, SUB_SET_RANGE, start, count, r, g, b]))
        return resp[2] == 0

    def set_all(self, r, g, b):
        resp = self._send(bytes([CMD, SUB_SET_ALL, r, g, b]))
        return resp[2] == 0

    def get_timing(self):
        resp = self._send(bytes([CMD, SUB_GET_TIMING]))
        lat_min  = struct.unpack_from("<I", resp, 3)[0]
        lat_max  = struct.unpack_from("<I", resp, 7)[0]
        lat_last = struct.unpack_from("<I", resp, 11)[0]
        calls    = struct.unpack_from("<I", resp, 15)[0]
        snapshot = struct.unpack_from("<I", resp, 19)[0]
        return {
            "latency_min": lat_min,
            "latency_max": lat_max,
            "latency_last": lat_last,
            "isr_calls": calls,
            "count_snapshot": snapshot,
        }

    def dump_buffer(self, offset, count=14):
        if count > 14:
            count = 14
        resp = self._send(bytes([
            CMD, SUB_DUMP_BUFFER,
            offset & 0xFF, (offset >> 8) & 0xFF,
            count
        ]))
        actual = resp[3]
        values = []
        for i in range(actual):
            val = struct.unpack_from("<H", resp, 4 + i * 2)[0]
            values.append(val)
        return values

    def freeze_rgb(self, enable):
        resp = self._send(bytes([CMD, SUB_FREEZE_RGB, 1 if enable else 0]))
        return resp[2] == 0


def cmd_set(args, kb):
    idx, r, g, b = int(args[0]), int(args[1]), int(args[2]), int(args[3])
    print(f"Setting {led_info(idx)} to RGB({r},{g},{b})")
    kb.set_led(idx, r, g, b)


def cmd_range(args, kb):
    start, count = int(args[0]), int(args[1])
    r, g, b = int(args[2]), int(args[3]), int(args[4])
    print(f"Setting LEDs {start}-{start+count-1} to RGB({r},{g},{b})")
    kb.set_range(start, count, r, g, b)


def cmd_all(args, kb):
    r, g, b = int(args[0]), int(args[1]), int(args[2])
    print(f"Setting all LEDs to RGB({r},{g},{b})")
    kb.set_all(r, g, b)


def cmd_timing(args, kb):
    t = kb.get_timing()
    period = 90  # timer ticks per WS2812 bit period
    print("ISR Timing Stats (since last read):")
    print(f"  Timer period:   {period} ticks (1.25 us)")
    print(f"  ISR calls:      {t['isr_calls']}")
    if t["latency_min"] == 0xFFFFFFFF:
        print("  (no ISR calls recorded)")
    else:
        print(f"  COUNT min:      {t['latency_min']} ticks")
        print(f"  COUNT max:      {t['latency_max']} ticks")
        print(f"  COUNT last:     {t['latency_last']} ticks")
        print(f"  Snapshot:       {t['count_snapshot']} ticks")
        if t["latency_max"] > period:
            print(f"  WARNING: max COUNT ({t['latency_max']}) > period ({period})!")
            print(f"           This means the ISR was delayed by at least "
                  f"{t['latency_max'] // period} full timer cycle(s).")


def cmd_dump(args, kb):
    offset = int(args[0])
    count = int(args[1]) if len(args) > 1 else 14
    values = kb.dump_buffer(offset, count)
    print(f"Frame buffer [{offset}..{offset + len(values) - 1}]:")
    for i, v in enumerate(values):
        idx = offset + i
        # Determine which LED/colour this bit belongs to
        if idx < LED_COUNT * BITS_PER_LED:
            led_idx = idx // BITS_PER_LED
            bit_in_led = idx % BITS_PER_LED
            byte_in_led = bit_in_led // 8
            # GRB order
            colour = ["G", "R", "B"][byte_in_led] if byte_in_led < 3 else "W"
            bit_in_byte = 7 - (bit_in_led % 8)
            info = f"LED {led_idx} {colour}[{bit_in_byte}]"
        else:
            info = "reset"
        marker = " *" if v not in (0, 25, 64) else ""
        print(f"  [{idx:4d}] = {v:3d}  ({info}){marker}")


def cmd_freeze(args, kb):
    enable = int(args[0]) if args else 1
    kb.freeze_rgb(enable)
    print(f"RGB matrix {'frozen' if enable else 'unfrozen'}")


def cmd_boundary_test(args, kb):
    """Light LEDs 40-46 one at a time in R, G, B to find batch boundary corruption."""
    print("=== Boundary Test ===")
    print("Tests LEDs around the DMA batch 1/2 boundary (bit 1024 = LED 42 blue byte)")
    print("RGB matrix will be frozen during test.\n")

    kb.freeze_rgb(True)
    time.sleep(0.1)

    test_leds = list(range(40, 47))
    colours = [("RED", 255, 0, 0), ("GREEN", 0, 255, 0), ("BLUE", 0, 0, 255)]

    for colour_name, r, g, b in colours:
        print(f"\n--- Testing {colour_name} ---")
        for led in test_leds:
            # Turn all off first
            kb.set_all(0, 0, 0)
            time.sleep(0.05)
            # Set single LED
            kb.set_led(led, r, g, b)
            info = led_info(led)
            input(f"  {info} → {colour_name}  [press Enter to continue]")

    kb.set_all(0, 0, 0)
    kb.freeze_rgb(False)
    print("\nDone — RGB matrix unfrozen.")


def cmd_sweep(args, kb):
    """Light LEDs one by one from 0 to 81 in red to visually find corruption start."""
    print("=== LED Sweep ===")
    print("Will light each LED one at a time in RED.")
    print("Press Enter to advance, or 'q' to quit.\n")

    kb.freeze_rgb(True)
    time.sleep(0.1)

    for led in range(LED_COUNT):
        kb.set_all(0, 0, 0)
        time.sleep(0.02)
        kb.set_led(led, 255, 0, 0)
        info = led_info(led)
        resp = input(f"  {info} → RED  [Enter/q] ")
        if resp.strip().lower() == 'q':
            break

    kb.set_all(0, 0, 0)
    kb.freeze_rgb(False)
    print("Done — RGB matrix unfrozen.")


def cmd_ping(args, kb):
    """Send VIA get_protocol_version (0x01) to test if RAW HID transport works."""
    print("Sending VIA get_protocol_version (0x01)...")
    try:
        resp = kb._send(bytes([0x01]))
        print(f"Response: {resp[:8].hex(' ')}")
        ver = (resp[1] << 8) | resp[2]
        print(f"VIA protocol version: {ver} (0x{ver:04x})")
        print("RAW HID transport is working!")
    except TimeoutError:
        print("No response — RAW HID transport is broken.")
    print()
    print("Now testing debug command 0xFE...")
    try:
        resp = kb._send(bytes([CMD, SUB_GET_TIMING]))
        print(f"Response: {resp[:8].hex(' ')}")
        if resp[0] == CMD:
            print("Debug interface is working!")
        else:
            print(f"Unexpected response cmd byte: 0x{resp[0]:02x}")
    except TimeoutError:
        print("No response to debug command — via_command_kb() override may not be linked.")


COMMANDS = {
    "ping":          (cmd_ping, 0, "ping"),
    "set":           (cmd_set, 4, "set <led> <r> <g> <b>"),
    "range":         (cmd_range, 5, "range <start> <count> <r> <g> <b>"),
    "all":           (cmd_all, 3, "all <r> <g> <b>"),
    "timing":        (cmd_timing, 0, "timing"),
    "dump":          (cmd_dump, 1, "dump <offset> [count]"),
    "freeze":        (cmd_freeze, 0, "freeze [0|1]"),
    "boundary-test": (cmd_boundary_test, 0, "boundary-test"),
    "sweep":         (cmd_sweep, 0, "sweep"),
}


def usage():
    print("Usage: ws2812_diag.py <command> [args...]\n")
    print("Commands:")
    for name, (_, _, help_text) in sorted(COMMANDS.items()):
        print(f"  {help_text}")
    sys.exit(1)


def main():
    if len(sys.argv) < 2:
        usage()

    cmd_name = sys.argv[1]
    if cmd_name not in COMMANDS:
        print(f"Unknown command: {cmd_name}")
        usage()

    func, min_args, _ = COMMANDS[cmd_name]
    args = sys.argv[2:]
    if len(args) < min_args:
        print(f"Not enough arguments for '{cmd_name}'")
        usage()

    kb = RT82Debug()
    try:
        kb.open()
        func(args, kb)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
    finally:
        kb.close()


if __name__ == "__main__":
    main()
