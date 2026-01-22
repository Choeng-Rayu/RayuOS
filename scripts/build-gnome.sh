#!/bin/bash
# RayuOS GNOME Edition Build Script
# Builds a lightweight GNOME desktop ISO using live-build

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build/gnome"
ARTIFACTS_DIR="${PROJECT_ROOT}/artifacts"
ARCH="${ARCH:-amd64}"
VERBOSE="${VERBOSE:-0}"

# Read version from VERSION file
if [ -f "$PROJECT_ROOT/VERSION" ]; then
    VERSION=$(cat "$PROJECT_ROOT/VERSION" | tr -d '[:space:]')
else
    VERSION="1.0.0"
fi

echo "========================================"
echo "  RayuOS GNOME Edition Builder"
echo "========================================"
echo "Version: $VERSION"
echo "Architecture: $ARCH"
echo "Build directory: $BUILD_DIR"
echo ""

# Check for root/sudo
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

# Check dependencies
for cmd in lb debootstrap xorriso; do
    if ! command -v $cmd &> /dev/null; then
        echo "Error: $cmd not found. Install with: apt install live-build debootstrap xorriso"
        exit 1
    fi
done

# Build C utilities first
echo "Building C utilities..."
if [ -d "$PROJECT_ROOT/src/rayuos-firstboot" ]; then
    make -C "$PROJECT_ROOT/src/rayuos-firstboot" clean all
fi
if [ -d "$PROJECT_ROOT/src/rayuos-sysinfo" ]; then
    make -C "$PROJECT_ROOT/src/rayuos-sysinfo" clean all
fi
if [ -d "$PROJECT_ROOT/src/rayu" ]; then
    make -C "$PROJECT_ROOT/src/rayu" clean all
fi

# Clean previous build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
mkdir -p "$ARTIFACTS_DIR"
cd "$BUILD_DIR"

# Initialize live-build config
echo "Configuring live-build..."
lb config \
    --distribution bookworm \
    --archive-areas "main contrib non-free non-free-firmware" \
    --architectures "$ARCH" \
    --binary-images iso-hybrid \
    --bootloaders "grub-efi,grub-pc" \
    --debian-installer none \
    --memtest none \
    --iso-application "RayuOS GNOME" \
    --iso-publisher "RayuOS Project" \
    --iso-volume "RAYUOS_GNOME" \
    --apt-recommends false \
    --apt-indices false \
    --firmware-chroot true

# Copy common configuration
echo "Applying common configuration..."
cp -r "$PROJECT_ROOT/configs/common/apt/"* config/archives/ 2>/dev/null || true
mkdir -p config/includes.chroot
cp -r "$PROJECT_ROOT/configs/common/includes.chroot/"* config/includes.chroot/ 2>/dev/null || true

# Copy GNOME-specific configuration
echo "Applying GNOME edition configuration..."
cp "$PROJECT_ROOT/configs/gnome/packages.list.chroot" config/package-lists/
mkdir -p config/hooks/live
cp "$PROJECT_ROOT/configs/gnome/hooks/live/"* config/hooks/live/ 2>/dev/null || true
chmod +x config/hooks/live/*.hook.chroot 2>/dev/null || true

# Copy built C utilities
if [ -f "$PROJECT_ROOT/src/rayuos-firstboot/rayuos-firstboot" ]; then
    mkdir -p config/includes.chroot/usr/local/bin
    cp "$PROJECT_ROOT/src/rayuos-firstboot/rayuos-firstboot" config/includes.chroot/usr/local/bin/
    chmod +x config/includes.chroot/usr/local/bin/rayuos-firstboot
fi
if [ -f "$PROJECT_ROOT/src/rayuos-sysinfo/rayuos-sysinfo" ]; then
    mkdir -p config/includes.chroot/usr/local/bin
    cp "$PROJECT_ROOT/src/rayuos-sysinfo/rayuos-sysinfo" config/includes.chroot/usr/local/bin/
    chmod +x config/includes.chroot/usr/local/bin/rayuos-sysinfo
fi
if [ -f "$PROJECT_ROOT/src/rayu/rayu" ]; then
    mkdir -p config/includes.chroot/usr/local/bin
    cp "$PROJECT_ROOT/src/rayu/rayu" config/includes.chroot/usr/local/bin/
    chmod +x config/includes.chroot/usr/local/bin/rayu
fi

# Build the ISO
echo ""
echo "Building ISO (this may take a while)..."
echo ""

if [ "$VERBOSE" = "1" ]; then
    lb build
else
    lb build 2>&1 | tee build.log
fi

# Move the ISO to artifacts
ISO_FILE=$(ls -1 *.iso 2>/dev/null | head -1)
if [ -n "$ISO_FILE" ]; then
    OUTPUT_NAME="rayuos-gnome-v${VERSION}-${ARCH}.iso"
    mv "$ISO_FILE" "$ARTIFACTS_DIR/$OUTPUT_NAME"
    echo ""
    echo "========================================"
    echo "  Build Complete!"
    echo "========================================"
    echo "ISO: $ARTIFACTS_DIR/$OUTPUT_NAME"
    echo "Size: $(du -h "$ARTIFACTS_DIR/$OUTPUT_NAME" | cut -f1)"
    echo ""
    echo "Test with: ./scripts/test-qemu.sh $ARTIFACTS_DIR/$OUTPUT_NAME --ram 2048"
else
    echo "Error: No ISO file produced"
    exit 1
fi

# Cleanup unless KEEP_BUILD is set
if [ "${KEEP_BUILD:-0}" != "1" ]; then
    echo "Cleaning up build directory..."
    cd "$PROJECT_ROOT"
    rm -rf "$BUILD_DIR"
fi

echo "Done!"
