# Maintainer: Florian Loitsch <florian@toit.io>
pkgname=shift_layout
pkgver=1.0.0
pkgrel=5
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
  "mouse-meta-toggle.c"
  "shift-layout-mouse.service"
  )
noextract=()
md5sums=('4cfa726b4f987f3f5877419faa5b2f9c'
         '340790b137f5fea2452a646d7c00be91'
         '7af99ffd1f0422cf4c37bcc88b79987e'
         '20258133f4319699fc37cab174f27830'
         'f60bb08fb582050c9e15f13f63cf283d'
         '2e8fb1852f69a6129c8bf7774b56c16d'
         'a64dcde4304d2e9eb0eba8bb944170c6')

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
	install -Dm755 mouse-meta-toggle \
		"$pkgdir/usr/lib/shift_layout/mouse-meta-toggle"
	install -Dm644 "$srcdir/shift-layout-mouse.service" \
		"$pkgdir/usr/lib/systemd/system/shift-layout-mouse.service"
}
