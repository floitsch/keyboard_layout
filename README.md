# Customized Keyboard Layout

This repository contains my files and notes on how to add a new keyboard layout
to my Linux setup.

## Steps
Use the PKGBUILD to install. Alternatively look at the package step in it.

You can switch to the layout with
```
setxkbmap us_shifted
```

Change the default layout:
```
sudo localectl --no-convert set-x11-keymap us_shifted,us dell101 ,dvorak
```

## Notes
KDE doesn't recognize the new layout.

## Links
https://medium.com/@damko/a-simple-humble-but-comprehensive-guide-to-xkb-for-linux-6f1ad5e13450
http://who-t.blogspot.com/2020/09/user-specific-xkb-configuration-putting.html
https://meesha.blog/2021/custom-keyboard-layout-in-x11-and-wayland.html
https://blog.stigok.com/2020/10/27/from-x11-xmodmap-to-wayland-xkb-custom-keyboard-layout.html
