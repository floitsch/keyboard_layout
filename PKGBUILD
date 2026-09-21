# Maintainer: Florian Loitsch <florian@toit.io>
pkgname=shift_layout
pkgver=1.0.0
pkgrel=8
pkgdesc="Customized keyboard layout shifting digits"
arch=('x86_64')
url="https://github.com/floitsch/keyboard_layout"
license=('Unlicense')
depends=('systemd' 'xkeyboard-config')
makedepends=()
provides=('us_shift_layout')
conflicts=('us_shift_layout')
replaces=('us_shift_layout')
install=shift_layout.install
source=(
  "us_shifted"
  "evdev_us_shifted.xml"
  "dv_shifted"
  "evdev_dv_shifted.xml"
  "90-custom-keyboard.hwdb"
  "90-shift-layout-dell-precision-5490.hwdb"
  "90-shift-layout-mouse.conf"
  "mouse-meta-toggle.c"
  "shift-layout-mouse.service"
  )
noextract=()
md5sums=('4cfa726b4f987f3f5877419faa5b2f9c'
         '340790b137f5fea2452a646d7c00be91'
         '7af99ffd1f0422cf4c37bcc88b79987e'
         '20258133f4319699fc37cab174f27830'
         'f60bb08fb582050c9e15f13f63cf283d'
         '431a0ce00ca6603746a5e26f29d0c590'
         '99d45043f950ba4e8f226bba6ae57583'
         'e3eba1f9bfbdba9acd354e299c825be8'
         'a117f6fdff5b3f790567d91630476657')

build() {
	cc $CPPFLAGS $CFLAGS -std=c11 -Wall -Wextra -Werror \
		-o mouse-meta-toggle "$srcdir/mouse-meta-toggle.c" $LDFLAGS
}

package() {
	install -Dm644 "$srcdir/us_shifted" \
		"$pkgdir/usr/share/xkeyboard-config-2/symbols/us_shifted"
	install -Dm644 "$srcdir/dv_shifted" \
		"$pkgdir/usr/share/xkeyboard-config-2/symbols/dv_shifted"
	install -Dm644 "$srcdir/evdev_us_shifted.xml" \
		"$pkgdir/usr/share/xkeyboard-config-2/rules/evdev_us_shifted.xml"
	install -Dm644 "$srcdir/evdev_dv_shifted.xml" \
		"$pkgdir/usr/share/xkeyboard-config-2/rules/evdev_dv_shifted.xml"
	install -Dm644 "$srcdir/90-custom-keyboard.hwdb" \
		"$pkgdir/etc/udev/hwdb.d/90-custom-keyboard.hwdb"
	install -Dm644 "$srcdir/90-shift-layout-dell-precision-5490.hwdb" \
		"$pkgdir/etc/udev/hwdb.d/90-shift-layout-dell-precision-5490.hwdb"
	install -Dm644 "$srcdir/90-shift-layout-mouse.conf" \
		"$pkgdir/usr/share/X11/xorg.conf.d/90-shift-layout-mouse.conf"
	install -Dm755 mouse-meta-toggle \
		"$pkgdir/usr/lib/shift_layout/mouse-meta-toggle"
	install -Dm644 "$srcdir/shift-layout-mouse.service" \
		"$pkgdir/usr/lib/systemd/system/shift-layout-mouse.service"
}
