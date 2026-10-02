# Customized keyboard layouts and mouse gestures

This repository contains shifted US and Dvorak keyboard layouts for Linux and
Android, device-specific Linux keyboard remaps, and a mouse gesture service for
KDE Wayland.

Both layouts put `!@#$%^&*<>` on the digit row without Shift and `1234567890`
with Shift. Shift-comma and Shift-period produce parentheses.

## Linux installation (Arch Linux)

From the repository root, build and install the package as a regular user
(with `base-devel` installed):

```sh
makepkg -si
```

The package installs:

- The `us_shifted` and `dv_shifted` XKB layouts and their XML fragments under
  `/usr/share/xkeyboard-config-2/`.
- The Carbon and Dell keyboard hwdb files under `/etc/udev/hwdb.d/`.
- The mouse proxy at `/usr/lib/shift_layout/mouse-meta-toggle` and the system
  unit `shift-layout-mouse.service`.

The install hook rebuilds the hardware database, reapplies input-device rules,
and **enables and starts the mouse service automatically**. Upgrades restart
it. Layout selection and KDE button-scrolling configuration are separate steps
below.

## Linux installation (Debian and Ubuntu)

The main Debian package contains the keyboard layouts and the Carbon and Dell
hwdb rules. The mouse proxy is a separate optional package. Build and install
them from the repository root:

```sh
sudo apt install debhelper
dpkg-buildpackage --no-sign --build=binary
sudo apt install ../shift-layout_1.0.0-2_all.deb
# Optional mouse gesture proxy:
sudo apt install ../shift-layout-mouse_1.0.0-2_*.deb
```

Installing through the package keeps all copied files tracked by `dpkg`. The
post-install hook rebuilds the hardware database and reapplies the input-device
rules automatically. The optional mouse package installs an enabled root system
service because exclusive input-device and `/dev/uinput` access cannot be
provided safely by a user service.

## Linux keyboard layouts (XKB)

For the current X11 session, select either layout:

```sh
setxkbmap us_shifted
# Or:
setxkbmap dv_shifted
```

For example, configure shifted US with standard Dvorak as a second layout in the
system X11 defaults:

```sh
sudo localectl --no-convert set-x11-keymap us_shifted,us dell101 ,dvorak
```

Use `dv_shifted` as the first layout instead if you want shifted Dvorak.
`--no-convert` leaves the console keymap unchanged.

`setxkbmap` is for X11. Configure the layout in the compositor for Wayland.
The package installs separate XML fragments but does not merge them into the
main XKB layout registry, so the layouts may not appear in KDE's layout picker.

### References

- [Adding a custom XKB layout](https://medium.com/@damko/a-simple-humble-but-comprehensive-guide-to-xkb-for-linux-6f1ad5e13450)
- [User-specific XKB configuration](http://who-t.blogspot.com/2020/09/user-specific-xkb-configuration-putting.html)
- [Custom keyboard layouts in X11 and Wayland](https://meesha.blog/2021/custom-keyboard-layout-in-x11-and-wayland.html)
- [From xmodmap to XKB on Wayland](https://blog.stigok.com/2020/10/27/from-x11-xmodmap-to-wayland-xkb-custom-keyboard-layout.html)

## Linux physical-key remaps (udev hwdb)

`90-custom-keyboard.hwdb` retains the original Carbon X1 remap. Its legacy
input-device match is also reported by other AT keyboards, so
`90-shift-layout-dell-precision-5490.hwdb` provides a later, DMI-scoped override
for the Dell Precision 5490. Both files can be installed safely: the Dell rule
does not match the Carbon, and it overrides every Carbon scan-code assumption
that differs on the Dell.

| Physical key | Carbon X1 result | Dell Precision 5490 result |
| --- | --- | --- |
| Escape | Caps Lock | Caps Lock |
| Backtick | Escape | Escape |
| Tab | Left Control | Left Control |
| Caps Lock | `=`/`+` in Dvorak | `=`/`+` in Dvorak |
| Fn | Backtick/tilde | Firmware-managed; no standalone event |
| Left Control | Left Meta | Backtick/tilde |
| Left Meta/Windows | Left Alt | Left Alt |
| Left Alt | Tab | Tab |
| Equals/plus | Backslash/pipe | Backslash/pipe |

The mouse entry matches USB `1ea7:0066` and restores the back button to
`BTN_SIDE`, replacing the direct Meta mapping used by older package versions.
The service now handles the mouse gestures.

Package installation applies these rules automatically. To install them
manually, or reapply them after editing:

```sh
sudo install -Dm644 90-custom-keyboard.hwdb /etc/udev/hwdb.d/90-custom-keyboard.hwdb
sudo install -Dm644 90-shift-layout-dell-precision-5490.hwdb \
  /etc/udev/hwdb.d/90-shift-layout-dell-precision-5490.hwdb
sudo systemd-hwdb update
sudo udevadm trigger --subsystem-match=input
```

Use `sudo evtest` to inspect a device's scan codes and key events before adapting
the keyboard match and mappings for another keyboard. See the
[ArchWiki scancode remapping guide](https://wiki.archlinux.org/title/Map_scancodes_to_keycodes#Remap_specific_device).

## Mouse gestures on KDE Wayland

The `shift-layout-mouse` system service proxies the `1ea7:0066` 2.4G Mouse
through virtual mouse and keyboard devices. Its back button behaves as follows:

- Click with little or no movement to toggle Left Meta (held until toggled off).
- Hold and move to the motion threshold to lock system-wide scrolling. Meta is
  released automatically when scrolling starts; releasing the button keeps
  scrolling active.
- Click again to leave scrolling mode.

The forward button and ordinary pointer, wheel, and button input pass through.
Stopping the service releases its exclusive grab so the physical mouse works
normally again. The proxy retries while the mouse is disconnected. On startup
and after a reconnect, it verifies that the selected event node provides pointer
motion and otherwise finds a matching motion-capable node from the same physical
interface.

### Install and start the service

On Arch Linux, `makepkg -si` installs and starts the service as described above.
To re-enable an already installed service:

```sh
sudo systemctl enable --now shift-layout-mouse.service
```

For a manual service-only installation on a Linux system with systemd, a C
compiler, and Linux input/uinput headers, run these commands from the repository
root:

```sh
build_dir=$(mktemp -d)
cc -std=c11 -Wall -Wextra -Werror -o "$build_dir/mouse-meta-toggle" mouse-meta-toggle.c
"$build_dir/mouse-meta-toggle" --self-test
sudo install -Dm755 "$build_dir/mouse-meta-toggle" /usr/lib/shift_layout/mouse-meta-toggle
sudo install -Dm644 shift-layout-mouse.service /etc/systemd/system/shift-layout-mouse.service
rm -r "$build_dir"
sudo systemctl daemon-reload
sudo systemctl enable --now shift-layout-mouse.service
```

If upgrading from the old direct-Meta mouse remap, also install and refresh the
hwdb file using the commands above. The service runs as root to access the
physical input device and `/dev/uinput`; it is a system unit, so do not use
`systemctl --user` for it. If the journal reports that `/dev/uinput` is missing,
load the module with `sudo modprobe uinput` and restart the service.

### Enable scrolling in KDE

The proxy holds `BTN_SIDE` (Linux input button 275) to request scrolling. KWin
must be configured to turn that button into scrolling on the virtual pointer.

#### X11

The packages install `90-shift-layout-mouse.conf`, which configures button 8
scrolling for the virtual `Shift Layout Mouse Proxy` on the next X server
start. To configure the current session without logging out, find the proxy and
set its libinput properties directly:

```sh
xinput list --short
xinput set-prop "Shift Layout Mouse Proxy" "libinput Scroll Method Enabled" 0 0 1
xinput set-prop "Shift Layout Mouse Proxy" "libinput Button Scrolling Button" 8
```

#### Wayland

The settings below use [KWin's input-device D-Bus properties](https://github.com/KDE/kwin/blob/master/src/backends/libinput/device.h).
Run these commands as your desktop user after the service is running and the
mouse is connected.

First list the pointer devices:

```sh
qdbus6 org.kde.KWin /org/kde/KWin/InputDevice \
  org.kde.KWin.InputDeviceManager.ListPointers
```

For a candidate event node, inspect its name and kernel path:

```sh
pointer=/org/kde/KWin/InputDevice/event9 # Replace event9 with a current event node.
busctl --user get-property org.kde.KWin "$pointer" org.kde.KWin.InputDevice name
udevadm info --query=path --name=/dev/input/"${pointer##*/}"
```

Choose `Shift Layout Mouse Proxy`; its kernel path also contains
`/devices/virtual/input/`. Then configure that pointer:

```sh
busctl --user set-property org.kde.KWin "$pointer" \
  org.kde.KWin.InputDevice scrollButton u 275
busctl --user set-property org.kde.KWin "$pointer" \
  org.kde.KWin.InputDevice scrollOnButtonDown b true
```

KWin persists these properties in `~/.config/kcminputrc`. Event numbers can
change after reboot or a service restart; rediscover the node before running
these commands again.

### Thin panel indicator (Plasma 5 and 6)

The **Mouse Mode Indicator** widget displays a small rectangular strip for the
mouse service: orange for locked Meta, blue for locked scrolling, dim gray when
inactive, and red when the mouse or service is unavailable. Hover for the status.
It updates every 100 ms without reading keyboard input or requiring desktop
administrator privileges.

After installing the updated mouse service, install the widget for your desktop:

```sh
sh plasma/install-widget.sh
```

The installer detects Plasma using the available `kpackagetool6` or
`kpackagetool5`; pass `--plasma-version 5` or `--plasma-version 6` to override.
Plasma 6 requires its `plasma5support` executable data engine.
The Arch package includes the Plasma 6 widget; the Debian build selects the
version using the build machine's package tool (defaults to Plasma 5).

Enter panel edit mode, add **Mouse Mode Indicator**, and drag it to the very top
of your left panel. In its configuration, adjust **Strip thickness** from 1 to
64 pixels (default 6). On vertical panels this sets its height, and the strip
fills the available panel width. On horizontal panels it sets the width instead.
Measurements follow desktop scaling; the panel theme may add surrounding spacing.
The strip keeps its space when inactive so other widgets do not move.

The service publishes its PID and mode atomically in
`/run/shift-layout-mouse/state`. Its runtime directory is readable by the desktop
and writable only by the root service; systemd removes it when the service stops.
The widget also checks that the publishing process still exists. Restart the
updated service to enable status publishing; an older service shows red.

### Configuration and troubleshooting

The default device is
`/dev/input/by-id/usb-1ea7_2.4G_Mouse-if01-event-mouse`. The second argument to
the executable is the motion threshold, defaulting to 12 accumulated raw motion
units across both axes. A larger value requires more movement to enter scrolling
mode.

To change the device path or threshold, create a systemd override:

```sh
sudo systemctl edit shift-layout-mouse.service
```

For example, set the threshold to 20 with:

```ini
[Service]
ExecStart=
ExecStart=/usr/lib/shift_layout/mouse-meta-toggle /dev/input/by-id/usb-1ea7_2.4G_Mouse-if01-event-mouse 20
```

Apply the change and inspect the service and its Meta/scroll state transitions:

```sh
sudo systemctl daemon-reload
sudo systemctl restart shift-layout-mouse.service
systemctl status shift-layout-mouse.service
journalctl -u shift-layout-mouse.service -b
```

An active service can still be waiting for the mouse. Look for the `proxying`
message in the journal to confirm it opened the device. If clicks toggle Meta
but movement does not scroll, check the KWin settings on the virtual pointer.

To stop the proxy and keep it disabled across reboots:

```sh
sudo systemctl disable --now shift-layout-mouse.service
```

A package upgrade enables and starts it again through the install hook.

## Android physical keyboard

The `android` directory builds a tiny, permission-free Android package that
provides two selectable physical-keyboard layouts:

- **US shifted + laptop key remaps**
- **Dvorak shifted + laptop key remaps**

The package uses Android's native Key Character Map overlay mechanism (the same
mechanism used by ExKeyMo). It is not an input method, has no user interface,
and does not run in the background. One overlay handles both layers: it maps the
physical keys first and then maps key/modifier combinations to characters.
The layouts intentionally omit Android's locale and layout-type hints so newer
Android versions offer both choices for every enabled input-method language.

### Build and install

Java 17 or newer, an Android SDK with platform 36, and `adb` are required.
Set `ANDROID_HOME` to the SDK directory or set `sdk.dir` in
`android/local.properties`. From this repository:

```sh
cd android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

The debug APK is self-signed and installable. On the Android device, connect the
Bluetooth keyboard and open **Settings → System → Keyboard → Physical keyboard**
(the path varies by vendor). Select the keyboard. On newer Android versions,
first select the active input-method language row, open **Layouts**, scroll past
the built-in layouts, and choose one of the entries from **Shifted layouts**.

The mappings reproduce the final effect of `90-custom-keyboard.hwdb` followed by
`us_shifted` or `dv_shifted`:

| Physical key | Android result |
| --- | --- |
| Escape | Caps Lock |
| Backtick | Escape |
| Tab | Left Control |
| Caps Lock | `]`/`}` in US, `=`/`+` in Dvorak |
| Fn, if reported as `KEY_FN` | Backtick/tilde |
| Left Control | Left Meta |
| Left Meta/Windows | Left Alt |
| Left Alt | Tab |
| Right bracket | Backslash/bar |

The digit row produces `!@#$%^&*<>` without Shift and `1234567890` with Shift.
Shift-comma and Shift-period produce parentheses.

### Check the Bluetooth keyboard's scan codes

Standard evdev scan codes are used for all ordinary keys. Fn keys are special:
Bluetooth keyboard firmware often consumes Fn and sends only the modified key,
so Android may never see a standalone Fn press. With USB debugging enabled, use
this while pressing Fn and the other remapped keys:

```sh
adb shell getevent -lt
```

The supplied layout maps evdev `KEY_FN` (decimal scan code 464) to backtick. If
Fn produces no event, that mapping cannot be implemented in an Android layout;
choose another physical key for `GRAVE`. If it produces a different `EV_KEY`
code, replace `464` in both `.kcm` files with that code converted from
hexadecimal to decimal, rebuild, and reinstall the APK.

The Android sources are in:

- `android/app/src/main/res/raw/us_shifted.kcm`
- `android/app/src/main/res/raw/dvorak_shifted.kcm`
- `android/app/src/main/res/xml/keyboard_layouts.xml`
