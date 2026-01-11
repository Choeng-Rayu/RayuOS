# Boot Issue Diagnosis and Fix

## Problem Summary

When testing the RayuOS CLI ISO in VirtualBox, the VM displayed a **black screen** with no visible output. The VM appeared to be running (network activity, disk reads), but nothing was displayed.

## Root Cause Analysis

### Investigation Steps

1. **Checked VirtualBox log** (`ra.log`):
   - No kernel panic or critical errors
   - VM successfully loaded and executed
   - BIOS detected bootable ISO correctly
   - Boot process started normally

2. **Mounted and inspected the ISO**:
   ```bash
   sudo mount -o loop artifacts/rayuos-cli-amd64.iso /tmp/iso_mount
   ```
   
   **Findings:**
   - ISO structure was correct
   - Kernel (`vmlinuz-6.1.0-41-amd64`) present - 7.9MB ✓
   - Initrd (`initrd.img-6.1.0-41-amd64`) present - 70MB ✓
   - Filesystem (`filesystem.squashfs`) present - 448MB ✓
   - GRUB bootloader installed correctly ✓

3. **Examined GRUB configuration** (`/boot/grub/grub.cfg`):
   ```grub
   menuentry "Live system (amd64)" --hotkey=l {
           linux   /live/vmlinuz-6.1.0-41-amd64 boot=live components quiet splash findiso=${iso_path}
           initrd  /live/initrd.img-6.1.0-41-amd64
   }
   ```

   **Problem identified:** The kernel boot parameter included `findiso=${iso_path}`, but the variable `${iso_path}` was **never defined** in the GRUB configuration.

### Root Cause

The `findiso` parameter is used by live-boot to locate the ISO file when booting from a hard disk with the ISO on it. However:

1. **The `${iso_path}` variable was undefined** in `config.cfg`
2. When GRUB expands `findiso=${iso_path}`, it becomes `findiso=` (empty value)
3. live-boot tries to find an ISO at an empty path, fails, and cannot mount the root filesystem
4. The system hangs with a black screen (no error message due to `quiet` parameter)

### Why This Happened

The live-build system automatically generates GRUB configurations that include `findiso=${iso_path}` for maximum compatibility. However, it expects the distribution to define this variable in the GRUB hooks. Our build didn't include this variable definition, causing the boot failure.

## The Fix

### Solution Implemented

Created a custom GRUB configuration file that:
1. **Removes the `findiso` parameter** (not needed when booting directly from ISO)
2. Uses simpler, more reliable boot parameters
3. Adds multiple boot options (normal, safe mode, debug)
4. Includes proper search directive to find the live system

### Fixed GRUB Configuration

**File:** `/home/rayu/RayuOS/configs/common/includes.binary/boot/grub/grub.cfg`

```grub
# Set default boot entry
set default=0
set timeout=5

# Search for the live system
search --no-floppy --set=root --file /live/vmlinuz-6.1.0-41-amd64

# Live boot entries
menuentry "RayuOS Live (amd64)" --hotkey=l {
        linux   /live/vmlinuz-6.1.0-41-amd64 boot=live components quiet splash
        initrd  /live/initrd.img-6.1.0-41-amd64
}

menuentry "RayuOS Live (amd64 - Safe Mode)" {
        linux   /live/vmlinuz-6.1.0-41-amd64 boot=live components noapic noapm nodma nomce nomodeset nosmp vga=normal
        initrd  /live/initrd.img-6.1.0-41-amd64
}

menuentry "RayuOS Live (amd64 - Debug Mode)" {
        linux   /live/vmlinuz-6.1.0-41-amd64 boot=live components debug
        initrd  /live/initrd.img-6.1.0-41-amd64
}
```

### Key Changes

| Before | After | Reason |
|--------|-------|--------|
| `findiso=${iso_path}` | *removed* | Variable was undefined, causing boot failure |
| No `search` command | `search --no-floppy --set=root --file /live/vmlinuz-6.1.0-41-amd64` | Automatically finds the correct boot device |
| Only 2 boot options | 3 boot options (Normal, Safe, Debug) | Better troubleshooting options |
| Generic menu titles | "RayuOS" branding | Better user experience |

## Testing the Fix

### Build Commands

```bash
# Remove old build
sudo rm -rf build/cli

# Rebuild with fixed GRUB configuration
sudo ./scripts/build-cli.sh

# New ISO will be at: artifacts/rayuos-cli-amd64.iso
```

### Expected Results

After rebuilding with the fix:
1. ✅ GRUB menu appears with 5 seconds timeout
2. ✅ "RayuOS Live (amd64)" option boots successfully
3. ✅ System displays boot messages (unless `quiet` is used)
4. ✅ Live system reaches login prompt
5. ✅ Login credentials: `live` / `live`
6. ✅ `rayu` command works

### Verification Steps

1. **Check GRUB config in new ISO:**
   ```bash
   sudo mount -o loop artifacts/rayuos-cli-amd64.iso /tmp/iso_mount
   cat /tmp/iso_mount/boot/grub/grub.cfg
   # Should show the fixed configuration
   sudo umount /tmp/iso_mount
   ```

2. **Test in VirtualBox:**
   - Create VM with 512MB RAM minimum
   - Attach the new ISO
   - Boot and verify GRUB menu appears
   - Select "RayuOS Live" and verify it boots to login

3. **If boot still fails:**
   - Try "Safe Mode" option
   - Try "Debug Mode" to see detailed boot messages
   - Check VirtualBox logs for new errors

## Technical Details

### live-boot Parameters

| Parameter | Purpose |
|-----------|---------|
| `boot=live` | Tells kernel this is a live system (no hard disk install) |
| `components` | Load live-boot components |
| `quiet` | Suppress most boot messages |
| `splash` | Show graphical boot splash (if available) |
| `findiso=<path>` | Search for ISO file at specified path (removed - not needed) |
| `noapic` | Disable Advanced Programmable Interrupt Controller (safe mode) |
| `nomodeset` | Disable kernel mode setting for graphics (safe mode) |
| `debug` | Enable verbose debugging output |

### GRUB search Command

```grub
search --no-floppy --set=root --file /live/vmlinuz-6.1.0-41-amd64
```

- `--no-floppy`: Don't search floppy drives (faster)
- `--set=root`: Set the root device variable
- `--file`: Search for a specific file to identify the correct device

This ensures GRUB finds the correct device containing the live system, whether it's a CD-ROM, USB drive, or virtual optical drive.

## Lessons Learned

1. **Always test ISOs in a VM** before announcing success
2. **GRUB variables must be defined** or not used at all
3. **Silent boot (`quiet`) hides errors** - use debug mode for troubleshooting
4. **live-build default configs assume things** - always customize for your needs
5. **VirtualBox logs are invaluable** for diagnosing boot issues

## Related Files

- **GRUB Config:** `/home/rayu/RayuOS/configs/common/includes.binary/boot/grub/grub.cfg`
- **Build Script:** `/home/rayu/RayuOS/scripts/build-cli.sh`
- **Troubleshooting:** `/home/rayu/RayuOS/docs/troubleshooting.md`
- **User Guide:** `/home/rayu/RayuOS/docs/user-guide.md`

## Status

- ✅ Issue diagnosed
- ✅ Fix implemented
- ✅ Documentation updated
- 🔄 ISO rebuild in progress
- ⏳ Testing pending

---

**Issue Date:** January 7, 2026  
**Fix Date:** January 11, 2026  
**Author:** GitHub Copilot AI Assistant
