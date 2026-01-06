# RayuOS Goals

## Vision

RayuOS is a lightweight, Debian-based Linux distribution designed to run efficiently on hardware with limited resources while still providing a modern, usable experience.

## Target Hardware

### CLI Edition
- **Minimum RAM**: 256 MB
- **Recommended RAM**: 512 MB
- **CPU**: Any x86_64 processor (including older single-core)
- **Storage**: 2 GB minimum
- **Use cases**: Servers, IoT devices, containers, old laptops, Raspberry Pi (future ARM support)

### GNOME Edition
- **Minimum RAM**: 1 GB
- **Recommended RAM**: 2 GB
- **CPU**: Dual-core x86_64 recommended
- **Storage**: 8 GB minimum
- **Use cases**: Desktop computing, lightweight laptops, budget computers

## Design Principles

### 1. Minimal by Default
- Only essential packages installed
- Services disabled unless necessary
- No automatic installation of Recommends

### 2. Performance First
- C language for custom utilities (no interpreted languages in critical paths)
- zram enabled by default for better memory utilization
- Aggressive journald limits to reduce disk I/O
- GNOME tuned for lower resource usage

### 3. Debian Compatibility
- Based on Debian Stable (Bookworm/12)
- Full access to Debian repositories
- Standard package management with apt
- No proprietary modifications to core system

### 4. User-Friendly
- Simple first-boot setup wizard
- Sensible defaults that work out of the box
- Clear documentation

## Non-Goals

- **Not a rolling release**: Stability over bleeding edge
- **Not for gaming**: No GPU driver optimization focus
- **Not enterprise**: No enterprise management features
- **Not a fork**: We use Debian packages, not custom rebuilds

## Success Metrics

1. CLI edition boots to login in under 10 seconds on modest hardware
2. GNOME edition idles under 500 MB RAM with no applications open
3. ISO size under 500 MB (CLI) / 1.5 GB (GNOME)
4. Builds reproducibly from this repository
