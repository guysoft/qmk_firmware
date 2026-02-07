#!/bin/bash
# Compile and flash script for Epomaker RT82 QMK firmware

set -e  # Exit on error

# ============================================================
# Environment Configuration
# ============================================================
HOME_DIR="$HOME"
WORKSPACE_DIR="$HOME_DIR/workspace/keyboards/rt82"
QMK_DIR="$WORKSPACE_DIR/Epomaker_RT82_QMK_Source_Code/qmk_firmware_Epomaker_RT82"
BUILD_DIR="$QMK_DIR/.build"

# Keyboard/keymap settings
KEYBOARD="epomaker/epomaker_rt82"
KEYMAP="${1:-default}"  # Pass keymap as first arg, default to "default"

# Binary output naming (QMK convention: slashes become underscores)
BINARY_NAME="epomaker_epomaker_rt82_${KEYMAP}.bin"
BINARY_PATH="$QMK_DIR/$BINARY_NAME"
HEX_PATH="$QMK_DIR/epomaker_epomaker_rt82_${KEYMAP}.hex"

# USB IDs (normal mode)
USB_VID="36b0"
USB_PID="30a3"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# ============================================================
# Helper Functions
# ============================================================
info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }
step()  { echo -e "${BLUE}[STEP]${NC} ${BOLD}$*${NC}"; }

usage() {
    echo "Usage: $0 [keymap] [command]"
    echo ""
    echo "Keymaps:  default, via"
    echo "Commands:"
    echo "  compile     Compile firmware only (default if no command given)"
    echo "  flash       Compile and flash firmware"
    echo "  flash-only  Flash previously compiled firmware (skip compile)"
    echo "  info        Show build info and USB device status"
    echo ""
    echo "Examples:"
    echo "  $0                    # Compile default keymap"
    echo "  $0 via compile        # Compile VIA keymap"
    echo "  $0 default flash      # Compile and flash default keymap"
    echo "  $0 via flash-only     # Flash previously compiled VIA firmware"
    echo ""
    exit 0
}

# ============================================================
# Parse Arguments
# ============================================================
# Handle --help / -h anywhere in args
for arg in "$@"; do
    if [[ "$arg" == "--help" || "$arg" == "-h" ]]; then
        usage
    fi
done

# If second arg is a known command, use it; otherwise default to "compile"
COMMAND="${2:-compile}"

# If first arg looks like a command (not a keymap name), adjust
case "$1" in
    compile|flash|flash-only|info)
        COMMAND="$1"
        KEYMAP="default"
        BINARY_NAME="epomaker_epomaker_rt82_${KEYMAP}.bin"
        BINARY_PATH="$QMK_DIR/$BINARY_NAME"
        HEX_PATH="$QMK_DIR/epomaker_epomaker_rt82_${KEYMAP}.hex"
        ;;
esac

# ============================================================
# Info Command
# ============================================================
do_info() {
    echo -e "${GREEN}=== Epomaker RT82 Build Info ===${NC}"
    echo ""
    echo "  QMK Dir:    $QMK_DIR"
    echo "  Keyboard:   $KEYBOARD"
    echo "  Keymap:     $KEYMAP"
    echo "  Binary:     $BINARY_NAME"
    echo ""

    if [ -f "$BINARY_PATH" ]; then
        local size
        size=$(stat -c%s "$BINARY_PATH" 2>/dev/null || stat -f%z "$BINARY_PATH" 2>/dev/null)
        local date
        date=$(stat -c%y "$BINARY_PATH" 2>/dev/null | cut -d. -f1 || stat -f%Sm "$BINARY_PATH" 2>/dev/null)
        info "Binary exists: $BINARY_NAME ($size bytes, $date)"
    else
        warn "Binary not found. Run compile first."
    fi

    echo ""
    step "USB device status:"
    if lsusb 2>/dev/null | grep -i "epomaker\|${USB_VID}:${USB_PID}\|36b0" ; then
        info "RT82 detected in normal mode"
    else
        warn "RT82 not detected via USB (may be in wireless mode or disconnected)"
    fi

    # Check for bootloader device
    echo ""
    step "Checking for bootloader/DFU device..."
    if lsusb 2>/dev/null | grep -iE "dfu|isp|boot|3443|eastsoft|essemi" ; then
        info "Bootloader device detected!"
    else
        warn "No bootloader device detected"
    fi
}

# ============================================================
# Compile
# ============================================================
do_compile() {
    echo -e "${GREEN}=== Epomaker RT82 Compile & Flash Script ===${NC}"
    echo ""
    info "Keyboard: $KEYBOARD"
    info "Keymap:   $KEYMAP"
    echo ""

    # Check QMK directory
    if [ ! -d "$QMK_DIR" ]; then
        error "QMK firmware directory not found at $QMK_DIR"
        exit 1
    fi

    # Check binary library exists
    if [ ! -f "$QMK_DIR/lib/rdr_lib/librdrcommon.a" ]; then
        error "Proprietary library not found at lib/rdr_lib/librdrcommon.a"
        error "This file is required for building (see BINARY_LIBRARY_NOTICE.md)"
        exit 1
    fi

    # Compile
    step "Compiling firmware..."
    cd "$QMK_DIR"

    if qmk compile -kb "$KEYBOARD" -km "$KEYMAP"; then
        echo ""
        info "Compilation successful!"
    else
        echo ""
        error "Compilation failed!"
        exit 1
    fi

    # Verify binary
    if [ ! -f "$BINARY_PATH" ]; then
        error "Binary file not found at $BINARY_PATH"
        error "Compilation may have produced output with unexpected name."
        echo "Files in QMK dir:"
        ls -la "$QMK_DIR"/*.bin 2>/dev/null || echo "  (none)"
        exit 1
    fi

    local size
    size=$(stat -c%s "$BINARY_PATH" 2>/dev/null || stat -f%z "$BINARY_PATH" 2>/dev/null)
    local flash_max=131072  # 128K flash
    info "Binary: $BINARY_NAME ($size bytes / $flash_max bytes flash, $(( size * 100 / flash_max ))% used)"

    if [ "$size" -gt "$flash_max" ]; then
        error "Binary exceeds flash size! ($size > $flash_max)"
        exit 1
    fi

    if [ -f "$HEX_PATH" ]; then
        info "Hex file also available: $(basename "$HEX_PATH")"
    fi
    echo ""
}

# ============================================================
# Flash
# ============================================================
do_flash() {
    if [ ! -f "$BINARY_PATH" ]; then
        error "Binary not found at $BINARY_PATH"
        error "Run compile first: $0 $KEYMAP compile"
        exit 1
    fi

    local size
    size=$(stat -c%s "$BINARY_PATH" 2>/dev/null || stat -f%z "$BINARY_PATH" 2>/dev/null)
    info "Firmware to flash: $BINARY_NAME ($size bytes)"
    echo ""

    step "Put the RT82 into bootloader/DFU mode"
    echo ""
    echo -e "  ${BOLD}Method 1: QMK Reset keycode${NC}"
    echo "    - If your current firmware has a RESET/QK_BOOT key mapped,"
    echo "      press it (often Fn+Esc or Fn+Backspace)"
    echo ""
    echo -e "  ${BOLD}Method 2: Bootmagic reset${NC}"
    echo "    - Unplug the keyboard"
    echo "    - Hold down the Escape key"
    echo "    - Plug the keyboard in while holding Escape"
    echo "    - Release Escape after 1 second"
    echo ""
    echo -e "  ${BOLD}Method 3: Hardware reset${NC}"
    echo "    - If accessible, press the physical reset button on the PCB"
    echo ""
    read -p "Press Enter when keyboard is in bootloader mode..."
    echo ""

    step "Detecting bootloader device..."

    # Give USB a moment to enumerate
    sleep 1

    # Try to detect the bootloader device
    local bootloader_found=false
    local lsusb_output
    lsusb_output=$(lsusb 2>/dev/null)

    # Check for common bootloader identifiers
    if echo "$lsusb_output" | grep -iE "dfu|isp|boot|3443" ; then
        bootloader_found=true
        info "Bootloader device found!"
    fi

    if [ "$bootloader_found" = false ]; then
        warn "Could not auto-detect bootloader device."
        echo ""
        echo "Current USB devices:"
        lsusb 2>/dev/null | head -20
        echo ""
        echo "If you see a new/unknown USB device above, note its VID:PID"
        echo "and update this script's flash command accordingly."
        echo ""
        read -p "Try to flash anyway? (y/N) " -r
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            error "Flash cancelled."
            exit 1
        fi
    fi

    echo ""
    step "Flashing firmware..."

    # -------------------------------------------------------
    # Flash method: Try multiple approaches
    # -------------------------------------------------------

    # Method 1: QMK flash (handles many bootloader types)
    info "Attempting: qmk flash..."
    cd "$QMK_DIR"
    if qmk flash -kb "$KEYBOARD" -km "$KEYMAP" 2>&1; then
        echo ""
        info "Flash successful!"
        echo ""
        info "Your RT82 should reboot with the new firmware."
        info "If the keyboard doesn't respond, unplug and replug it."
        return 0
    fi

    warn "qmk flash did not succeed. Trying alternative methods..."
    echo ""

    # Method 2: dfu-util (common for ARM DFU bootloaders)
    if command -v dfu-util &>/dev/null; then
        info "Attempting: dfu-util..."
        if dfu-util -a 0 -D "$BINARY_PATH" -s 0x00000000:leave 2>&1; then
            echo ""
            info "Flash successful via dfu-util!"
            return 0
        fi
        warn "dfu-util did not succeed."
    fi

    # Method 3: dfu-programmer
    if command -v dfu-programmer &>/dev/null; then
        info "Attempting: dfu-programmer..."
        # Note: dfu-programmer needs the MCU type, which may not match
        warn "dfu-programmer may not support the FS026 MCU."
    fi

    echo ""
    error "Automatic flashing failed."
    echo ""
    echo -e "${YELLOW}Manual flash options:${NC}"
    echo ""
    echo "  1. Use the Epomaker flashing tool (Windows):"
    echo "     - Download from Epomaker support website"
    echo "     - Load $BINARY_NAME in the tool"
    echo "     - Follow on-screen instructions"
    echo ""
    echo "  2. Use the EastSoft ISP tool (if available for FS026):"
    echo "     - Connect keyboard in bootloader mode"
    echo "     - Select the .bin or .hex file"
    echo "     - Program and verify"
    echo ""
    echo "  3. Binary file location:"
    echo "     $BINARY_PATH"
    if [ -f "$HEX_PATH" ]; then
        echo "     $HEX_PATH"
    fi
    echo ""
    return 1
}

# ============================================================
# Main
# ============================================================
case "$COMMAND" in
    compile)
        do_compile
        info "Done! To flash, run: $0 $KEYMAP flash"
        ;;
    flash)
        do_compile
        do_flash
        ;;
    flash-only)
        do_flash
        ;;
    info)
        do_info
        ;;
    *)
        error "Unknown command: $COMMAND"
        usage
        ;;
esac
