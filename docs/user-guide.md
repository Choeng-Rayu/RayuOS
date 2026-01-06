# RayuOS User Guide

## Table of Contents
1. [System Requirements](#system-requirements)
2. [Testing in VirtualBox](#testing-in-virtualbox)
3. [Installation Guide](#installation-guide)
4. [Using the Rayu Command](#using-the-rayu-command)
5. [System Monitoring](#system-monitoring)
6. [Network Configuration](#network-configuration)
7. [Tips & Tricks](#tips--tricks)

---

## System Requirements

### CLI Edition (Minimal)
- **Processor**: 1 GHz x86_64 (64-bit) CPU
- **RAM**: 256 MB minimum, 512 MB recommended
- **Storage**: 4 GB minimum, 8 GB recommended
- **Boot**: UEFI or Legacy BIOS

### GNOME Edition (Desktop)
- **Processor**: 1.5 GHz dual-core x86_64 (64-bit) CPU
- **RAM**: 1 GB minimum, 2 GB recommended
- **Storage**: 10 GB minimum, 20 GB recommended
- **Graphics**: Any GPU with 256 MB VRAM
- **Boot**: UEFI or Legacy BIOS

---

## Testing in VirtualBox

### Step 1: Install VirtualBox
If you don't have VirtualBox installed:
```bash
# On Ubuntu/Debian
sudo apt update
sudo apt install virtualbox

# On other systems, download from:
# https://www.virtualbox.org/wiki/Downloads
```

### Step 2: Create a New Virtual Machine

1. **Open VirtualBox** and click "New"

2. **Configure VM Settings:**
   - **Name**: RayuOS CLI (or RayuOS GNOME)
   - **Type**: Linux
   - **Version**: Debian (64-bit)
   - **Click**: Next

3. **Memory (RAM):**
   - CLI Edition: 512 MB minimum, 1024 MB recommended
   - GNOME Edition: 2048 MB minimum, 4096 MB recommended
   - **Click**: Next

4. **Hard Disk:**
   - Select "Create a virtual hard disk now"
   - **Click**: Create
   - Choose "VDI (VirtualBox Disk Image)"
   - **Click**: Next
   - Select "Dynamically allocated"
   - **Click**: Next
   - Set disk size:
     - CLI Edition: 8 GB
     - GNOME Edition: 20 GB
   - **Click**: Create

### Step 3: Configure VM Settings

1. **Select your VM** and click "Settings"

2. **System Settings:**
   - Go to "System" → "Motherboard"
   - Enable "Enable EFI (special OSes only)" (optional)
   - Boot Order: Optical, Hard Disk

3. **Storage Settings:**
   - Go to "Storage"
   - Click on "Empty" under Controller: IDE
   - Click the disk icon on the right
   - Choose "Choose a disk file..."
   - Navigate to: `/home/rayu/RayuOS/artifacts/rayuos-cli-amd64.iso`
     (or `rayuos-gnome-amd64.iso` for GNOME edition)
   - **Click**: OK

4. **Network Settings (Optional):**
   - Go to "Network"
   - Adapter 1: Attached to "NAT" or "Bridged Adapter"

5. **Click**: OK

### Step 4: Start the Virtual Machine

1. **Select your VM** and click "Start"

2. **Wait for boot** - You'll see the RayuOS boot menu

3. **Select boot option:**
   - Use arrow keys to select "RayuOS Live"
   - Press Enter

4. **Wait for system to boot** (30-60 seconds)

5. **Login credentials:**
   - **Username**: `live` (or `root` for full access)
   - **Password**: `live` (or `toor` for root)

### Step 5: Test the System

Once logged in, try these commands:
```bash
# Show system monitoring
rayu

# Check system information
rayuos-sysinfo

# Test network
ip addr

# Test package management (updates package list)
rayu update

# Search for packages
rayu search htop

# Check CPU and memory
htop
```

---

## Installation Guide

### Live Boot vs Installation

RayuOS currently runs as a **Live System** - meaning it runs directly from the ISO without installation. Your changes will **not persist** after reboot unless you:

1. Install to a physical drive (manual installation)
2. Use persistence mode (advanced)

### Manual Installation to Hard Drive

For permanent installation, you can manually install to a partition:

```bash
# 1. Become root
sudo -i

# 2. Identify your target partition
lsblk
# Example: /dev/sda1 is your target

# 3. Format the partition (WARNING: This erases all data!)
mkfs.ext4 /dev/sda1

# 4. Mount the partition
mount /dev/sda1 /mnt

# 5. Copy the live system
rsync -avx --exclude=/proc/* --exclude=/sys/* --exclude=/dev/* \
      --exclude=/tmp/* --exclude=/run/* --exclude=/mnt/* \
      / /mnt/

# 6. Prepare for chroot
mount --bind /dev /mnt/dev
mount --bind /proc /mnt/proc
mount --bind /sys /mnt/sys

# 7. Enter the new system
chroot /mnt

# 8. Install GRUB bootloader
# For BIOS systems:
grub-install /dev/sda
update-grub

# For UEFI systems:
apt install grub-efi-amd64
grub-install --target=x86_64-efi --efi-directory=/boot/efi
update-grub

# 9. Set root password
passwd root

# 10. Create a user
adduser yourusername
usermod -aG sudo yourusername

# 11. Exit and reboot
exit
umount -R /mnt
reboot
```

---

## Using the Rayu Command

The `rayu` command is RayuOS's custom package management and system monitoring tool.

### Basic Syntax
```bash
rayu [command] [arguments]
```

### Available Commands

#### System Monitoring (Default)
```bash
# Show current system status with visual progress bars
rayu

# Output shows:
# - CPU usage with color-coded bars
# - RAM usage (Used/Total)
# - Disk usage for all mounted partitions
# - Network interfaces
# - Installed packages count
```

#### Package Management

```bash
# Update package lists
rayu update
sudo rayu update

# Upgrade all packages
rayu upgrade
sudo rayu upgrade

# Install a package
rayu install [package-name]
sudo rayu install vim

# Install multiple packages
sudo rayu install vim git curl

# Remove a package
rayu remove [package-name]
sudo rayu remove package-name

# Search for packages
rayu search [keyword]
rayu search editor

# Show package information
rayu show [package-name]
rayu show vim
```

#### System Information

```bash
# Show detailed system info
rayu info

# Output includes:
# - RayuOS version
# - Linux kernel version
# - Hostname
# - Uptime
# - CPU model
# - Total RAM
# - Architecture
```

#### Live Monitoring

```bash
# Continuous monitoring (updates every 2 seconds)
rayu monitor

# Press Ctrl+C to exit
```

### Color Indicators

The `rayu` command uses color-coded progress bars:
- **Green**: 0-50% usage (healthy)
- **Yellow**: 50-80% usage (moderate)
- **Red**: 80-100% usage (high)

---

## System Monitoring

### Built-in Tools

RayuOS includes several monitoring utilities:

#### 1. Rayu (Simple)
```bash
rayu
```
Quick overview with visual bars for CPU, RAM, disk, and network.

#### 2. RayuOS System Info
```bash
rayuos-sysinfo
```
Detailed system information report.

#### 3. Htop (Advanced)
```bash
htop
```
Interactive process viewer with CPU, RAM, and process details.

#### 4. Traditional Commands
```bash
# CPU info
lscpu

# Memory usage
free -h

# Disk usage
df -h

# Network interfaces
ip addr

# System load
uptime
```

---

## Network Configuration

### Using NetworkManager

RayuOS uses NetworkManager for network management.

#### Check Network Status
```bash
# List network interfaces
ip addr

# Check NetworkManager status
systemctl status NetworkManager

# List available networks (WiFi)
nmcli device wifi list
```

#### Connect to WiFi
```bash
# Scan for networks
nmcli device wifi rescan

# Connect to a network
nmcli device wifi connect "SSID" password "your-password"

# Example:
nmcli device wifi connect "MyHomeWiFi" password "mypassword123"
```

#### Wired Connection
```bash
# Wired (Ethernet) connections usually work automatically
# Check connection
nmcli connection show

# Restart a connection
nmcli connection down "Wired connection 1"
nmcli connection up "Wired connection 1"
```

---

## Tips & Tricks

### 1. Save Your Work
Since RayuOS runs live, any files you create will be lost on reboot unless you:
- Save to an external USB drive
- Mount a partition and save there
- Use network storage (SSH, FTP, etc.)

```bash
# Mount a USB drive
mkdir /mnt/usb
mount /dev/sdb1 /mnt/usb
cp myfile.txt /mnt/usb/
umount /mnt/usb
```

### 2. Optimize Performance

```bash
# Check zram (compressed swap in RAM)
zramctl

# View memory usage
free -h

# Clear page cache (if needed)
sudo sync
sudo echo 3 > /proc/sys/vm/drop_caches
```

### 3. Customize Your Session

```bash
# Edit bash aliases
nano ~/.bashrc

# Add custom aliases:
alias ll='ls -lah'
alias update='sudo rayu update && sudo rayu upgrade'

# Apply changes
source ~/.bashrc
```

### 4. Check System Logs

```bash
# View system logs
journalctl -xe

# Follow logs in real-time
journalctl -f

# Check boot messages
dmesg | less
```

### 5. Package Management Best Practices

```bash
# Always update before installing
sudo rayu update

# Search before installing
rayu search package-name

# Clean up after installation
sudo apt autoremove
sudo apt clean
```

### 6. Keyboard Shortcuts (GNOME Edition)

- **Super (Windows) Key**: Open Activities
- **Super + L**: Lock screen
- **Alt + F2**: Run command
- **Ctrl + Alt + T**: Open terminal
- **Super + Arrow Keys**: Snap windows

### 7. Performance on Low-RAM Systems

RayuOS is optimized for low RAM, but you can further optimize:

```bash
# Check what's using memory
ps aux --sort=-%mem | head -10

# Disable unnecessary services (if you know what you're doing)
sudo systemctl disable service-name
```

### 8. Rescue and Recovery

If you need to recover files or fix a broken system:

1. Boot RayuOS live
2. Mount your hard drive partition
3. Access your files or fix configuration

```bash
# Mount your system partition
sudo mount /dev/sda1 /mnt

# Access files
cd /mnt/home/username

# Fix GRUB from live system
sudo mount /dev/sda1 /mnt
sudo mount --bind /dev /mnt/dev
sudo mount --bind /proc /mnt/proc
sudo mount --bind /sys /mnt/sys
sudo chroot /mnt
grub-install /dev/sda
update-grub
exit
```

---

## Getting Help

### Documentation
- Build Guide: `docs/build.md`
- Goals & Design: `docs/goals.md`
- README: `README.md`

### Command Help
```bash
# Built-in help
rayu --help
rayuos-sysinfo --help

# Manual pages
man systemctl
man nmcli
man apt
```

### System Information
```bash
# Check your RayuOS version
cat /etc/os-release

# Check kernel version
uname -a

# Check installed packages
rayu show-installed
dpkg -l | wc -l
```

---

## Troubleshooting

### Boot Issues

**Problem**: VM won't boot from ISO
- **Solution**: Check VM boot order (Storage → Optical should be first)
- Enable/disable EFI mode in VM settings

**Problem**: Black screen after boot
- **Solution**: Wait 60 seconds, press Enter
- Try adding `nomodeset` to boot parameters

### Network Issues

**Problem**: No network connection
```bash
# Check if NetworkManager is running
systemctl status NetworkManager

# Restart NetworkManager
sudo systemctl restart NetworkManager

# Check interfaces
ip link show
```

### Performance Issues

**Problem**: System feels slow
```bash
# Check CPU usage
rayu

# Check processes
htop

# Check if swap is working
free -h
swapon -s
```

### Package Manager Issues

**Problem**: "Unable to locate package"
```bash
# Update package lists first
sudo rayu update

# Or directly:
sudo apt update
```

---

## Advanced Usage

### Creating Persistence

To save changes across reboots, create a persistence partition:

```bash
# On a USB drive with RayuOS ISO
# Create a second partition labeled "persistence"
# Format as ext4
# Create persistence.conf:
echo "/ union" > /mnt/persistence/persistence.conf
```

### Custom Boot Parameters

Edit GRUB menu at boot:
- Press 'e' to edit boot entry
- Add parameters to the linux line:
  - `nomodeset` - Disable KMS (graphics issues)
  - `acpi=off` - Disable ACPI (old hardware)
  - `mem=512M` - Limit RAM usage
- Press Ctrl+X or F10 to boot

---

## Quick Reference Card

| Command | Description |
|---------|-------------|
| `rayu` | Show system monitoring |
| `rayu update` | Update package lists |
| `rayu install PKG` | Install package |
| `rayu remove PKG` | Remove package |
| `rayu search TERM` | Search packages |
| `rayu monitor` | Live monitoring |
| `rayuos-sysinfo` | Detailed system info |
| `htop` | Process monitor |
| `ip addr` | Show network interfaces |
| `df -h` | Show disk usage |
| `free -h` | Show RAM usage |

---

**Enjoy RayuOS!** 🚀

For issues or contributions, visit: https://github.com/yourusername/RayuOS
