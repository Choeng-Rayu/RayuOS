# RayuOS Troubleshooting Guide

## Common Issues and Solutions

### 1. ISO Won't Boot in VirtualBox (Black Screen)

**Symptoms:**
- VM starts but shows black screen
- No GRUB menu appears
- VM appears to be running but nothing displays

**Cause:**
The GRUB configuration had an undefined variable `${iso_path}` in the `findiso` parameter, causing the boot process to fail silently.

**Solution:**
This issue has been fixed in the latest build. The GRUB configuration now uses a simpler boot parameter without `findiso`.

**If you built before the fix:**
1. Delete the old build: `sudo rm -rf build/cli`
2. Rebuild the ISO: `sudo ./scripts/build-cli.sh`
3. Use the new ISO from `artifacts/rayuos-cli-amd64.iso`

**Workaround for existing ISO:**
If you can't rebuild, try these boot parameters at GRUB prompt:
```
c  # Enter GRUB command line
set root=(cd0)
linux /live/vmlinuz-6.1.0-41-amd64 boot=live components
initrd /live/initrd.img-6.1.0-41-amd64
boot
```

---

### 2. GRUB Configuration Debugging

**Check GRUB config in ISO:**
```bash
# Mount the ISO
mkdir -p /tmp/iso_mount
sudo mount -o loop artifacts/rayuos-cli-amd64.iso /tmp/iso_mount

# View GRUB configuration
cat /tmp/iso_mount/boot/grub/grub.cfg
cat /tmp/iso_mount/boot/grub/config.cfg

# Check live system files
ls -lh /tmp/iso_mount/live/

# Unmount
sudo umount /tmp/iso_mount
```

**Expected files in `/live/`:**
- `vmlinuz-6.1.0-41-amd64` - Linux kernel (~8MB)
- `initrd.img-6.1.0-41-amd64` - Initial RAM disk (~70MB)
- `filesystem.squashfs` - Compressed root filesystem (~450MB)
- `filesystem.packages` - List of installed packages

---

### 3. VirtualBox Settings Issues

**Problem: VM won't boot at all**
- **Check boot order**: Settings → System → Motherboard → Boot Order
  - Optical should be first
  - Enable "Hard Disk" second
- **Try both BIOS modes**:
  - Disable "Enable EFI" for Legacy BIOS boot
  - Enable "Enable EFI" for UEFI boot
- **Increase timeout**: VM may need more time to read ISO

**Problem: Very slow boot**
- Increase RAM allocation (512MB minimum, 1GB recommended)
- Enable VT-x/AMD-V in host BIOS
- Check "Enable PAE/NX" in VM settings
- Use "VMSVGA" or "VBoxVGA" graphics controller

---

### 4. Live System Won't Start

**Symptoms:**
- GRUB menu appears
- Boot starts but hangs or drops to emergency shell

**Common Causes:**
1. **Incomplete ISO build** - Check build logs for errors
2. **Corrupted download** - Re-download or rebuild
3. **Insufficient RAM** - Need at least 256MB for CLI

**Debug Mode Boot:**
1. Select "RayuOS Live (amd64 - Debug Mode)" from GRUB menu
2. Or edit boot parameters and remove `quiet splash`
3. Watch boot messages for errors

**Check ISO integrity:**
```bash
# In VirtualBox, select "Utilities → Verify integrity" from GRUB menu
# Or manually:
sudo mount -o loop artifacts/rayuos-cli-amd64.iso /tmp/iso_mount
cd /tmp/iso_mount
sha256sum -c sha256sum.txt
cd -
sudo umount /tmp/iso_mount
```

---

### 5. Network Issues in Live System

**No network connection:**
```bash
# Check NetworkManager status
systemctl status NetworkManager

# Restart NetworkManager
sudo systemctl restart NetworkManager

# Check interfaces
ip addr
nmcli device status

# Manual network configuration
sudo nmcli device wifi connect "SSID" password "PASSWORD"
```

**Can't reach internet:**
```bash
# Test connectivity
ping -c 3 8.8.8.8  # Google DNS
ping -c 3 debian.org

# Check DNS
cat /etc/resolv.conf

# Restart networking
sudo systemctl restart NetworkManager
```

---

### 6. Build Errors

**Error: "E: Unable to locate package"**
```bash
# Update package lists
sudo apt update

# Check your internet connection
ping -c 3 deb.debian.org

# Try different mirror
# Edit configs/common/archives/debian.list.chroot
```

**Error: "Package not found: networkmanager"**
- Should be `network-manager` (with hyphen)
- This is fixed in the current configuration

**Build fails with permission errors:**
```bash
# Ensure you're running with sudo
sudo ./scripts/build-cli.sh

# Clean and retry
sudo rm -rf build/cli
sudo ./scripts/build-cli.sh
```

**Disk space issues:**
```bash
# Check available space (need ~3GB for CLI, ~5GB for GNOME)
df -h /home

# Clean old builds
sudo rm -rf build/*/
rm -rf artifacts/*.iso

# Clean apt cache
sudo apt clean
```

---

### 7. Custom Utilities Not Working

**rayu command not found:**
```bash
# Rebuild C utilities
cd src/rayu && make clean && make
cd ../rayuos-sysinfo && make clean && make
cd ../rayuos-firstboot && make clean && make
cd ../..

# Rebuild ISO
sudo rm -rf build/cli
sudo ./scripts/build-cli.sh
```

**rayu shows no output:**
```bash
# Check if rayu is installed
which rayu
ls -la /usr/local/bin/rayu

# Test directly
/usr/local/bin/rayu

# Check system files it reads
ls -la /proc/meminfo /proc/stat /proc/cpuinfo
```

---

### 8. Package Management Issues

**apt update fails:**
```bash
# Check network first
ping -c 3 deb.debian.org

# Check sources list
cat /etc/apt/sources.list
cat /etc/apt/sources.list.d/*

# Try with rayu
sudo rayu update
```

**Can't install packages:**
```bash
# Make sure you have network
ip addr

# Update first
sudo rayu update

# Try apt directly
sudo apt update
sudo apt install <package-name>

# Check disk space
df -h /
```

---

### 9. Performance Problems

**System feels slow:**
```bash
# Check resource usage
rayu
htop

# Check zram (compressed swap)
zramctl
free -h

# Check for heavy processes
top
ps aux --sort=-%mem | head -10
```

**High CPU usage:**
```bash
# Find the culprit
top -o %CPU
ps aux --sort=-%cpu | head -10

# Check for updates
sudo rayu upgrade
```

**Running out of RAM:**
```bash
# Check memory usage
free -h
rayu

# zram should be active
zramctl

# Reduce memory usage
systemctl stop apt-daily.service
systemctl stop apt-daily-upgrade.service
```

---

### 10. Recovery and Rescue

**Boot to recovery mode:**
1. Reboot VM
2. At GRUB menu, select "RayuOS Live (amd64 - Safe Mode)"
3. This disables APIC, DMA, MCE, and other features

**Mount host filesystem from live:**
```bash
# Identify partitions
lsblk
sudo fdisk -l

# Mount a partition
sudo mkdir -p /mnt/host
sudo mount /dev/sda1 /mnt/host

# Access files
ls /mnt/host/home/

# Unmount when done
sudo umount /mnt/host
```

**Reset to defaults:**
Since RayuOS Live doesn't persist changes, simply reboot to reset everything.

---

## Getting Help

### Before Asking for Help

1. **Check the logs:**
   ```bash
   # System logs
   journalctl -xe
   
   # Boot messages
   dmesg | less
   
   # Service status
   systemctl status
   ```

2. **Gather system info:**
   ```bash
   rayuos-sysinfo > system-info.txt
   uname -a >> system-info.txt
   lsb_release -a >> system-info.txt
   ```

3. **Test in safe mode:**
   - Try booting with "Safe Mode" option
   - This helps isolate hardware issues

### Reporting Issues

When reporting issues, include:
- RayuOS edition (CLI or GNOME)
- VM software and version (VirtualBox 7.x)
- VM settings (RAM, CPU, etc.)
- Error messages or screenshots
- Output of `rayuos-sysinfo`
- Build log if build failed

### Log Files to Check

```bash
# View all logs
sudo journalctl -b

# Live-build logs (on build machine)
ls -la /home/rayu/RayuOS/build/cli/log/
cat /home/rayu/RayuOS/build-cli.log

# VirtualBox logs (on host)
# Check VirtualBox VM folder for VBox.log file
```

---

## Quick Diagnostic Commands

```bash
# System health check
rayu                    # System monitoring
rayuos-sysinfo          # Detailed system info
htop                    # Process monitor
free -h                 # Memory usage
df -h                   # Disk usage
ip addr                 # Network interfaces

# Service status
systemctl status NetworkManager
systemctl status systemd-journald
systemctl --failed      # Show failed services

# Hardware info
lscpu                   # CPU info
lsblk                   # Block devices
lspci                   # PCI devices
lsusb                   # USB devices

# Network diagnostics
nmcli device status     # Network status
ping -c 3 8.8.8.8      # Internet connectivity
ip route                # Routing table
```

---

## Known Issues

1. **GRUB findiso parameter** - Fixed in current builds
2. **zstd warning in initramfs** - Harmless, uses gzip instead
3. **VBoxGuestPropSvc error** - Normal, Guest Additions not installed in live
4. **First boot may be slow** - Normal, caching filesystem

---

## Additional Resources

- [Build Guide](build.md) - How to build RayuOS
- [User Guide](user-guide.md) - How to use RayuOS
- [Goals Document](goals.md) - Design goals and specifications
- [README](../README.md) - Project overview

---

**Last Updated:** January 11, 2026
