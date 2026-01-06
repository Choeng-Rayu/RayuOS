# Building RayuOS

## Prerequisites

### Build Host Requirements

- Debian 12 (Bookworm) or Ubuntu 22.04+ recommended
- At least 10 GB free disk space
- Root/sudo access for live-build

### Install Build Dependencies

```bash
sudo apt update
sudo apt install -y \
    live-build \
    debootstrap \
    squashfs-tools \
    xorriso \
    grub-pc-bin \
    grub-efi-amd64-bin \
    mtools \
    dosfstools \
    gcc \
    make \
    git
```

## Building the ISO

### CLI Edition

```bash
# From repository root
./scripts/build-cli.sh

# Output: artifacts/rayuos-cli-amd64.iso
```

### GNOME Edition

```bash
# From repository root
./scripts/build-gnome.sh

# Output: artifacts/rayuos-gnome-amd64.iso
```

### Build Options

Both build scripts accept environment variables:

```bash
# Custom architecture (default: amd64)
ARCH=i386 ./scripts/build-cli.sh

# Keep build directory for debugging
KEEP_BUILD=1 ./scripts/build-gnome.sh

# Verbose output
VERBOSE=1 ./scripts/build-cli.sh
```

## Testing

### Test in QEMU

```bash
# Test CLI edition with 512MB RAM
./scripts/test-qemu.sh artifacts/rayuos-cli-amd64.iso

# Test GNOME edition with 2GB RAM
./scripts/test-qemu.sh artifacts/rayuos-gnome-amd64.iso --ram 2048

# Test with specific CPU cores
./scripts/test-qemu.sh artifacts/rayuos-cli-amd64.iso --cpus 1 --ram 512
```

### Install QEMU (if needed)

```bash
sudo apt install qemu-system-x86 qemu-utils ovmf
```

## Building C Utilities

The C utilities are built automatically during ISO creation, but you can build them manually:

```bash
# Build all utilities
cd src/rayuos-firstboot && make && cd ../..
cd src/rayuos-sysinfo && make && cd ../..

# Install locally for testing
sudo make -C src/rayuos-firstboot install
sudo make -C src/rayuos-sysinfo install
```

## Customization

### Adding Packages

Edit the package lists in `configs/<edition>/packages.list.chroot`:

```
# One package per line
package-name
another-package
```

### Custom Hooks

Add scripts to `configs/<edition>/hooks/live/` or `configs/<edition>/hooks/normal/`:

- `hooks/live/*.chroot` - Run in chroot during build
- `hooks/normal/*.chroot` - Run during normal (installed) boot

### Branding

Edit files in `configs/common/includes.chroot/`:

- `/etc/os-release` - OS identification
- `/etc/issue` - Login prompt text
- `/etc/motd` - Message of the day

## Troubleshooting

### Build fails with permission error

```bash
# Run with sudo or fix permissions
sudo ./scripts/build-cli.sh
```

### Out of disk space

```bash
# Clean previous builds
rm -rf build/
rm -rf artifacts/*.iso
```

### Package not found

```bash
# Update package lists in build config
# Check configs/common/apt/sources.list.chroot
```

## CI/CD

GitHub Actions workflow is provided in `.github/workflows/build.yml` for automated builds on tags.
