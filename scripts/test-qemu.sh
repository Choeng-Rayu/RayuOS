#!/bin/bash
# RayuOS QEMU Test Script
# Boot an ISO in QEMU with configurable resources

set -e

# Default values (low resource testing)
RAM="${RAM:-1024}"
CPUS="${CPUS:-2}"
DISK_SIZE="${DISK_SIZE:-8G}"
EFI="${EFI:-1}"

usage() {
    echo "Usage: $0 <iso-file> [options]"
    echo ""
    echo "Options:"
    echo "  --ram <MB>      RAM in megabytes (default: 1024)"
    echo "  --cpus <N>      Number of CPUs (default: 2)"
    echo "  --disk <size>   Virtual disk size (default: 8G)"
    echo "  --no-efi        Use BIOS instead of UEFI"
    echo "  --install       Create a persistent disk for installation"
    echo "  --help          Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 artifacts/rayuos-cli-amd64.iso --ram 512 --cpus 1"
    echo "  $0 artifacts/rayuos-gnome-amd64.iso --ram 2048"
    exit 1
}

# Parse arguments
ISO_FILE=""
INSTALL_MODE=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --ram)
            RAM="$2"
            shift 2
            ;;
        --cpus)
            CPUS="$2"
            shift 2
            ;;
        --disk)
            DISK_SIZE="$2"
            shift 2
            ;;
        --no-efi)
            EFI=0
            shift
            ;;
        --install)
            INSTALL_MODE=1
            shift
            ;;
        --help|-h)
            usage
            ;;
        -*)
            echo "Unknown option: $1"
            usage
            ;;
        *)
            if [ -z "$ISO_FILE" ]; then
                ISO_FILE="$1"
            fi
            shift
            ;;
    esac
done

# Validate ISO file
if [ -z "$ISO_FILE" ]; then
    echo "Error: No ISO file specified"
    usage
fi

if [ ! -f "$ISO_FILE" ]; then
    echo "Error: ISO file not found: $ISO_FILE"
    exit 1
fi

# Check for QEMU
if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Error: qemu-system-x86_64 not found"
    echo "Install with: sudo apt install qemu-system-x86"
    exit 1
fi

echo "========================================"
echo "  RayuOS QEMU Test"
echo "========================================"
echo "ISO: $ISO_FILE"
echo "RAM: ${RAM}MB"
echo "CPUs: $CPUS"
echo "Boot: $([ "$EFI" = "1" ] && echo "UEFI" || echo "BIOS")"
echo ""

# Build QEMU command
QEMU_CMD=(
    qemu-system-x86_64
    -m "$RAM"
    -smp "$CPUS"
    -cdrom "$ISO_FILE"
    -boot d
    -enable-kvm
    -cpu host
    -vga virtio
    -display gtk
    -device virtio-net-pci,netdev=net0
    -netdev user,id=net0
    -device virtio-rng-pci
)

# Add EFI firmware if requested
if [ "$EFI" = "1" ]; then
    # Try to find OVMF firmware
    OVMF_PATHS=(
        "/usr/share/OVMF/OVMF_CODE.fd"
        "/usr/share/ovmf/OVMF.fd"
        "/usr/share/edk2-ovmf/x64/OVMF_CODE.fd"
    )
    
    OVMF_FOUND=""
    for path in "${OVMF_PATHS[@]}"; do
        if [ -f "$path" ]; then
            OVMF_FOUND="$path"
            break
        fi
    done
    
    if [ -n "$OVMF_FOUND" ]; then
        QEMU_CMD+=(-bios "$OVMF_FOUND")
        echo "Using UEFI: $OVMF_FOUND"
    else
        echo "Warning: OVMF not found, falling back to BIOS"
        echo "Install UEFI support with: sudo apt install ovmf"
    fi
fi

# Add virtual disk for install mode
if [ "$INSTALL_MODE" = "1" ]; then
    DISK_FILE="${ISO_FILE%.iso}-disk.qcow2"
    if [ ! -f "$DISK_FILE" ]; then
        echo "Creating virtual disk: $DISK_FILE ($DISK_SIZE)"
        qemu-img create -f qcow2 "$DISK_FILE" "$DISK_SIZE"
    fi
    QEMU_CMD+=(-hda "$DISK_FILE")
    echo "Virtual disk: $DISK_FILE"
fi

echo ""
echo "Starting QEMU..."
echo "(Press Ctrl+Alt+G to release mouse, Ctrl+Alt+F to toggle fullscreen)"
echo ""

# Run QEMU
"${QEMU_CMD[@]}"
