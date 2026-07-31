# Maintainer: Florian Loitsch <florian@toit.io>
pkgname=shift_layout
pkgver=1.0.0
pkgrel=4
pkgdesc="Customized keyboard layout shifting digits"
arch=('any')
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
  )
noextract=()
md5sums=('4cfa726b4f987f3f5877419faa5b2f9c'
         '340790b137f5fea2452a646d7c00be91'
         '7af99ffd1f0422cf4c37bcc88b79987e'
         '20258133f4319699fc37cab174f27830'
         'dcf14306c7d8d78e9d7ca5840dd8f240')

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
}
