# RayuOS

<div align="center">

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

### ⚡ **Lightweight • Fast • Minimalist** ⚡

<br>

<a href="https://drive.google.com/uc?export=download&confirm=1&id=1kp0vxle0xflPgcAgtFnrBG5DGMbt4fhi">
  <img src="https://img.shields.io/badge/⬇️_Download-CLI_Edition-FF6B35?style=for-the-badge&logo=gnu-bash&logoColor=white" height="50">
</a>
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="https://drive.google.com/uc?export=download&confirm=1&id=1kp0vxle0xflPgcAgtFnrBG5DGMbt4fhi">
  <img src="https://img.shields.io/badge/⬇️_Download-Gnome_GUI-4ECDC4?style=for-the-badge&logo=gnome&logoColor=white" height="50">
</a>

<br><br>

<p align="center">
  <strong>A lightweight Debian-based Linux distribution optimized for low RAM and CPU usage</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/License-MIT-blue?style=flat-square" alt="License">
  <img src="https://img.shields.io/badge/Platform-Linux-orange?style=flat-square" alt="Platform">
  <img src="https://img.shields.io/badge/Base-Debian-red?style=flat-square" alt="Base">
</p>

## Editions

| Edition | Target | Min RAM | Description |
|---------|--------|---------|-------------|
| **CLI** | Servers, IoT, old hardware | 256 MB | Minimal command-line system |
| **GNOME** | Desktop, laptops | 1 GB | Lightweight GNOME desktop |

## ✨ Features

- 🚀 **Lightweight**: Optimized for systems with limited resources  
- 🐧 **Debian-based**: Stable foundation with access to Debian repositories  
- 🔧 **C utilities**: Custom system tools written in C for maximum efficiency  
- 📦 **Minimal bloat**: Careful package selection, no unnecessary services  
- 💾 **zram support**: Compressed swap for better memory utilization  
- ⚡ **Fast boot**: Designed for quick startup and responsiveness

## Version Management

The version is defined in the `VERSION` file at the project root. To update the version:

```bash
echo "1.1.0" > VERSION
```

When building ISOs, the version is automatically included in the filename:
- CLI: `rayuos-cli-v1.0.0-amd64.iso`
- GNOME: `rayuos-gnome-v1.0.0-amd64.iso`

---

## 🔨 Quick Start

### Prerequisites (Build Host)

```bash
# Debian/Ubuntu
sudo apt install live-build debootstrap squashfs-tools xorriso grub-pc-bin grub-efi-amd64-bin mtools
```

### Build Options

**Build CLI Edition:**
```bash
./scripts/build-cli.sh
```

**Build GNOME Edition:**
```bash
./scripts/build-gnome.sh
```

**Test in QEMU:**
```bash
./scripts/test-qemu.sh artifacts/rayuos-cli-amd64.iso
```

---

## 📥 Download Pre-built ISO

| Edition | Size | ISO Name |
|---------|------|----------|
| **CLI** | ~800 MB | `rayuos-cli-v1.0.0-amd64.iso` |
| **GNOME** | ~1.5 GB | `rayuos-gnome-v1.0.0-amd64.iso` |

> 🔗 **Download all ISOs**: [RayuOS Releases](https://drive.google.com/uc?export=download&confirm=1&id=1kp0vxle0xflPgcAgtFnrBG5DGMbt4fhi)

---

## 📖 Installation & Usage Guide

RayuOS comes in two editions: **CLI** (command-line) and **GNOME** (desktop). Choose your path below:

### 🖥️ CLI Edition - Getting Started

### Step 1: Download and Verify the ISO
2. Verify the ISO file integrity:
   ```bash
   ls -lh rayuos-cli-amd64.iso
   ```

### Step 2: Create a Bootable USB Drive

#### On Linux:

1. Insert a USB drive (minimum 2 GB)
2. Identify the USB device:
   ```bash
   lsblk
   ```
   Note the device name (e.g., `/dev/sdb`)

3. Write the ISO to the USB drive:
   ```bash
   sudo dd if=rayuos-cli-amd64.iso of=/dev/sdX bs=4M status=progress
   sudo sync
   ```
   Replace `sdX` with your USB device

4. Safely eject the USB:
   ```bash
   sudo eject /dev/sdX
   ```

#### On macOS:

1. Insert USB drive and find its identifier:
   ```bash
   diskutil list
   ```

2. Unmount the drive:
   ```bash
   diskutil unmountDisk /dev/diskX
   ```

3. Write ISO to USB:
   ```bash
   sudo dd if=rayuos-cli-amd64.iso of=/dev/rdiskX bs=4m
   ```

4. Eject:
   ```bash
   diskutil ejectDisk /dev/diskX
   ```

#### On Windows:

- Use **Rufus** or **Etcher**: https://www.balena.io/etcher/
- Select the ISO file and your USB drive
- Click "Flash" to write the image

### Step 3: Boot from USB

1. Insert the USB drive into your computer
2. Restart the computer and enter BIOS/UEFI (usually F2, F10, Del, or Esc during startup)
3. Change boot order to USB first
4. Save and exit - the system will boot from USB

### Step 4: Installation Process

1. At the boot menu, select "Live" or the default option
2. Wait for the system to load (this may take 1-2 minutes)
3. Login with:
   - **User**: `root`
   - **Password**: (as configured, typically empty on live boot)

### Step 5: Using RayuOS CLI

Once booted, you have access to a lightweight command-line interface:

#### Common Commands:

```bash
# System information
rayuos-sysinfo                    # Display system information
uname -a                          # Kernel information
cat /etc/os-release               # OS details

# Package management
apt update                        # Update package lists
apt install package-name          # Install packages
apt remove package-name           # Remove packages
apt upgrade                       # Upgrade packages

# System management
systemctl status                  # Check services
systemctl start service-name      # Start a service
systemctl stop service-name       # Stop a service

# Network configuration
ip addr show                      # Display network interfaces
nmcli                            # NetworkManager CLI tool
```

#### Installation to Hard Drive:

1. Create partitions (using `parted` or `cfdisk`):
   ```bash
   sudo cfdisk /dev/sdX
   ```

2. Format partitions:
   ```bash
   sudo mkfs.ext4 /dev/sdX1      # Root partition
   sudo mkswap /dev/sdX2         # Swap partition
   ```

3. Use the `rayuos-firstboot` utility to complete installation:
   ```bash
   sudo rayuos-firstboot
   ```

4. Follow the on-screen prompts to install to your hard drive

---

### 🎨 GNOME Edition - Getting Started

The GNOME edition provides a user-friendly desktop environment. Follow these steps:

### Step 1: Download and Create Bootable Media

Follow **Steps 1-2** from the CLI section above, but use the **GNOME ISO** instead.

### Step 2: Boot the GNOME Live Environment

1. Insert USB drive and restart
2. Select boot from USB (see CLI Step 3)
3. Wait for the GNOME desktop to load (this may take 2-3 minutes)

### Step 3: Initial Login

Once the desktop loads:

1. You will see the GNOME login screen
2. Click on the user or select **"Not listed?"** to login as:
   - **User**: `root`
   - **Password**: (as configured, typically empty)

3. Click **Sign In**

### Step 4: GNOME Desktop Overview

Once logged in, you'll see the GNOME desktop with:

- **Activities** (top-left): Access applications and search
- **Taskbar** (bottom): Application dock and system tray
- **Application Menu** (top-right): System settings and user menu

### Step 5: Essential GNOME Applications

#### Opening Applications:

1. Click **Activities** in the top-left corner
2. Search for the application name (e.g., "Files", "Terminal")
3. Click the application to open it

#### Key Applications:

| Application | Purpose |
|-------------|---------|
| **Files** | File manager for browsing directories |
| **Terminal** | Command-line access within the desktop |
| **Text Editor** | Edit text and configuration files |
| **Settings** | Configure system preferences |
| **Software** | Install/remove applications |
| **Firefox** (if included) | Web browsing |

### Step 6: Common Tasks in GNOME

#### Opening a Terminal:

1. Click **Activities** → Search for "Terminal"
2. Click **Terminal** to open
3. You now have a command-line interface within the desktop

#### Connecting to Network:

1. Click the network icon (top-right)
2. Select your Wi-Fi network
3. Enter password if required
4. Connection status will show as connected

#### Installing Software:

**Method 1: Using Software Application**
1. Click **Activities** → Search "Software"
2. Click **Software**
3. Search for desired package
4. Click **Install** button

**Method 2: Using Terminal (faster)**
```bash
sudo apt update
sudo apt install package-name
```

#### File Management:

1. Open **Files** application
2. Navigate using the sidebar or address bar
3. Right-click files/folders for context menu options:
   - Copy
   - Cut
   - Delete
   - Rename
   - Properties

### Step 7: System Settings

1. Click the system menu (top-right) → **Settings**
2. Or: Click **Activities** → Search "Settings"

#### Important Settings:

- **Wi-Fi & Network**: Configure internet connection
- **Display**: Adjust resolution and scaling
- **Power**: Battery and power settings
- **Users**: Manage user accounts
- **Date & Time**: Set timezone and time
- **Keyboard**: Change keyboard layout
- **Sound**: Configure audio

### Step 8: Installation to Hard Drive

1. Double-click **Install to Disk** (if available on desktop)
2. Or open **Terminal** and run:
   ```bash
   sudo rayuos-firstboot
   ```

3. Follow the installation wizard:
   - Select target disk
   - Create/select partitions
   - Confirm installation
   - Wait for process to complete
   - Reboot when prompted

4. Remove USB drive and boot from hard drive

### Step 9: Post-Installation Setup

After installing to your hard drive:

1. **Install Additional Software** (optional):
   ```bash
   sudo apt install firefox gimp vlc  # Example packages
   ```

2. **Configure System**:
   - Set timezone: `Settings → Date & Time`
   - Configure network: `Settings → Wi-Fi & Network`
   - Add user accounts: `Settings → Users`

3. **Update System**:
   ```bash
   sudo apt update
   sudo apt upgrade
   ```

### Troubleshooting GNOME

| Issue | Solution |
|-------|----------|
| **System is slow** | Close unnecessary applications, check RAM usage in GNOME System Monitor |
| **Cannot connect to Wi-Fi** | Click network icon → Select network → Enter password |
| **Application won't open** | Open Terminal and run the app to see error messages |
| **Keyboard/Mouse not responding** | Try plugging into different USB port, or restart |
| **Screen resolution wrong** | Go to `Settings → Display` and select correct resolution |

### Performance Tips for GNOME Edition

Since RayuOS GNOME is optimized for low-resource systems:

1. **Close Unused Applications**: Each open app consumes memory
2. **Disable Visual Effects**: `Settings → Appearance → Turn off effects`
3. **Monitor Resources**: Open Terminal and run `top` or `htop`
4. **Clean Package Cache**:
   ```bash
   sudo apt clean
   sudo apt autoclean
   ```

---

---

## 📁 Project Structure

```
RayuOS/
├── configs/
│   ├── common/              # Shared configuration
│   ├── cli/                 # CLI edition config
│   └── gnome/               # GNOME edition config
├── docs/
│   ├── goals.md             # Project goals
│   └── build.md             # Build instructions
├── src/
│   ├── rayuos-firstboot/    # First-boot setup utility (C)
│   └── rayuos-sysinfo/      # System info utility (C)
├── scripts/
│   ├── build-cli.sh         # Build CLI ISO
│   ├── build-gnome.sh       # Build GNOME ISO
│   └── test-qemu.sh         # Test ISO in QEMU
└── artifacts/               # Built ISOs (gitignored)
```

## 📚 Documentation

- 📋 [Project Goals](docs/goals.md)
- 🔨 [Build Instructions](docs/build.md)
- 🐛 [Troubleshooting](docs/troubleshooting.md)
- 👤 [User Guide](docs/user-guide.md)

---

## 📜 License

MIT License - see [LICENSE](LICENSE) for details.

## 🤝 Contributing

Contributions are welcome! Please read the documentation first, then submit pull requests.

---

<p align="center">
  <strong>Made with ❤️ for minimal systems</strong>
</p>