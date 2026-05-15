#!/bin/bash
# Setup script for the cross-platform apt binary
# Detects platform and configures sources accordingly

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TARGET="${SCRIPT_DIR}/apt"

echo "=== Cross-Platform APT Setup ==="
echo ""

# Check if the binary exists
if [ ! -f "$TARGET" ]; then
    echo "Building apt binary..."
    make -C "$SCRIPT_DIR"
fi

echo "Binary: $TARGET"
file "$TARGET"
echo ""

# Detect platform
if [ -d "/data/data/com.termux" ]; then
    echo "Detected: Termux/Android environment"
    
    TERMUX_PREFIX="/data/data/com.termux/files/usr"
    APT_ETC="${TERMUX_PREFIX}/etc/apt"
    SOURCES_D="${APT_ETC}/sources.list.d"
    
    mkdir -p "$SOURCES_D"
    
    if [ ! -f "${SOURCES_D}/termux.sources" ]; then
        echo "Installing Termux APT sources config..."
        cp "${SCRIPT_DIR}/termux.sources" "${SOURCES_D}/termux.sources"
    else
        echo "Termux sources already configured."
    fi
    
    # Verify the configuration
    echo ""
    echo "APT configuration for Termux:"
    echo "  Config: ${APT_ETC}/apt.conf"
    echo "  Sources: ${SOURCES_D}/"
    echo "  Status: ${TERMUX_PREFIX}/var/lib/dpkg/status"
    
    # Create necessary directories if missing
    mkdir -p "${TERMUX_PREFIX}/var/lib/apt/lists"
    mkdir -p "${TERMUX_PREFIX}/var/cache/apt/archives/partial"
    mkdir -p "${TERMUX_PREFIX}/var/log/apt"
    
    echo "Directories created/verified."
else
    echo "Detected: PC Linux environment"
    echo ""
    echo "The binary will use the system's APT configuration."
    echo "Sources: /etc/apt/sources.list"
    echo ""
    echo "To use Termux repos on PC Linux, configure sources manually:"
    echo "  cp ${SCRIPT_DIR}/termux.sources /etc/apt/sources.list.d/"
fi

echo ""
echo "Setup complete!"
echo "Try: ${TARGET} --help"
