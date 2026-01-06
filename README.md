# RayuOS
```
  _____                      ____   _____ 
 |  __ \                    / __ \ / ____|
 | |__) |__ _ _   _ _   _  | |  | | (___  
 |  _  // _` | | | | | | | | |  | |\___ \ 
 | | \ \ (_| | |_| | |_| | | |__| |____) |
 |_|  \_\__,_|\__, |\__,_|  \____/|_____/ 
               __/ |                      
              |___/
```

A lightweight Debian-based Linux distribution optimized for low RAM and CPU usage.

## Editions

| Edition | Target | Min RAM | Description |
|---------|--------|---------|-------------|
| **CLI** | Servers, IoT, old hardware | 256 MB | Minimal command-line system |
| **GNOME** | Desktop, laptops | 1 GB | Lightweight GNOME desktop |

## Features

- 🚀 **Lightweight**: Optimized for systems with limited resources
- 🐧 **Debian-based**: Stable foundation with access to Debian repositories
- 🔧 **C utilities**: Custom system tools written in C for maximum efficiency
- 📦 **Minimal bloat**: Careful package selection, no unnecessary services
- 💾 **zram support**: Compressed swap for better memory utilization

## Quick Start

### Prerequisites (Build Host)

```bash
# Debian/Ubuntu
sudo apt install live-build debootstrap squashfs-tools xorriso grub-pc-bin grub-efi-amd64-bin mtools
```

### Build CLI Edition

```bash
./scripts/build-cli.sh
```

### Build GNOME Edition

```bash
./scripts/build-gnome.sh
```

### Test in QEMU

```bash
./scripts/test-qemu.sh artifacts/rayuos-cli-amd64.iso
```

## Project Structure

```
RayuOS/
├── configs/
│   ├── common/          # Shared configuration
│   ├── cli/             # CLI edition config
│   └── gnome/           # GNOME edition config
├── docs/
│   ├── goals.md         # Project goals
│   └── build.md         # Build instructions
├── src/
│   ├── rayuos-firstboot/   # First-boot setup utility (C)
│   └── rayuos-sysinfo/     # System info utility (C)
├── scripts/
│   ├── build-cli.sh     # Build CLI ISO
│   ├── build-gnome.sh   # Build GNOME ISO
│   └── test-qemu.sh     # Test ISO in QEMU
└── artifacts/           # Built ISOs (gitignored)
```

## Documentation

- [Project Goals](docs/goals.md)
- [Build Instructions](docs/build.md)

## License

MIT License - see [LICENSE](LICENSE) for details.

## Contributing

Contributions welcome! Please read the documentation first, then submit pull requests.