# Maintainer: Florian Loitsch <florian@toit.io>
pkgname=us_shift_layout
pkgver=1.0.0
pkgrel=1
pkgdesc="Customized keyboard layout shifting digits"
arch=('x86_64')
url="https://github.com/floitsch/keyboard_layout"
license=('Unlicense')
depends=()
makedepends=()
source=("us_shifted" "evdev_us_shifted.xml")
noextract=()
md5sums=('4cfa726b4f987f3f5877419faa5b2f9c'
         '340790b137f5fea2452a646d7c00be91')

package() {
	mkdir -p "$pkgdir/usr/share/X11/xkb/symbols/"
	mkdir -p "$pkgdir/usr/share/X11/xkb/rules/"
	cp "$srcdir/us_shifted" "$pkgdir/usr/share/X11/xkb/symbols/"
	cp "$srcdir/evdev_us_shifted.xml" "$pkgdir/usr/share/X11/xkb/rules/"
}
