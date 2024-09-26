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
