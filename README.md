# Customized Keyboard Layout

This repository contains my files and notes on how to add a new keyboard layout
to my Linux setup.

## xkbmap

Some minor changes, like shifting numbers and moving parens.

### Steps
Use the PKGBUILD to install. Alternatively look at the package step in it.

You can switch to the layout with
```
setxkbmap us_shifted
```

Change the default layout:
```
sudo localectl --no-convert set-x11-keymap us_shifted,us dell101 ,dvorak
```

### Notes
KDE doesn't recognize the new layout.

### Links
https://medium.com/@damko/a-simple-humble-but-comprehensive-guide-to-xkb-for-linux-6f1ad5e13450
http://who-t.blogspot.com/2020/09/user-specific-xkb-configuration-putting.html
https://meesha.blog/2021/custom-keyboard-layout-in-x11-and-wayland.html
https://blog.stigok.com/2020/10/27/from-x11-xmodmap-to-wayland-xkb-custom-keyboard-layout.html

## udev
Moves the escape, tab, caps lock, ... using udev rules.

`sudo evtest` to find scan values and actions

### Steps
These are already run by the PKGBUILD.
```
sudo cp 90-custom-keyboard.hwdb /etc/udev/hwdb.d
sudo systemd-hwdb update
sudo udevadm trigger
```

### Links
https://wiki.archlinux.org/title/Map_scancodes_to_keycodes#Remap_specific_device

## Mouse gestures on KDE Wayland

The `shift-layout-mouse` system service proxies the `1ea7:0066` 2.4G Mouse
through virtual mouse and keyboard devices. Its back button behaves as follows:

- Click without moving to toggle Left Meta.
- Hold and move beyond a small threshold to lock system-wide scrolling. Meta is
  released automatically when scrolling starts.
- Click again to leave scrolling mode.

The forward button and all other mouse input pass through unchanged. If the
service is stopped, its exclusive grab is released and the physical mouse works
normally again. Some receivers expose several event nodes under the same
`event-mouse` symlink. On startup and after a reconnect, the proxy verifies that
the selected node provides pointer motion and otherwise finds the matching
motion-capable node from the same physical interface.

KWin identifies the scrolling button as `BTN_SIDE` (Linux input button 275).
Enable button scrolling for the current device with:

```sh
busctl --user set-property org.kde.KWin \
  /org/kde/KWin/InputDevice/event9 org.kde.KWin.InputDevice \
  scrollButton u 275
busctl --user set-property org.kde.KWin \
  /org/kde/KWin/InputDevice/event9 org.kde.KWin.InputDevice \
  scrollOnButtonDown b true
```

KWin persists both properties in `~/.config/kcminputrc`. The `event9` name can
change after reboot and the proxy adds another pointer device. Find the current
devices with:

```sh
qdbus6 org.kde.KWin /org/kde/KWin/InputDevice \
  org.kde.KWin.InputDeviceManager.ListPointers
```

The movement threshold is the last argument in
`shift-layout-mouse.service` and defaults to 12 raw motion units. Check the
service and its current Meta/scroll state transitions with:

```sh
systemctl status shift-layout-mouse.service
journalctl -u shift-layout-mouse.service
```
