# Maintainer: Florian Loitsch <florian@toit.io>
pkgname=shift_layout
pkgver=1.0.0
pkgrel=9
pkgdesc="Customized keyboard layout shifting digits"
arch=('x86_64')
url="https://github.com/floitsch/keyboard_layout"
license=('Unlicense')
depends=('systemd' 'xkeyboard-config')
makedepends=()
optdepends=('plasma-workspace: Mouse Mode Indicator panel widget'
            'plasma5support: Plasma 6 widget status reader')
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
  "plasma/org.floitsch.shiftlayout.status/metadata.json"
  "plasma/org.floitsch.shiftlayout.status/contents/ui/main.qml"
  "plasma/org.floitsch.shiftlayout.status/contents/ui/configGeneral.qml"
  "plasma/org.floitsch.shiftlayout.status/contents/config/main.xml"
  "plasma/org.floitsch.shiftlayout.status/contents/config/config.qml"
  )
noextract=()
md5sums=('4cfa726b4f987f3f5877419faa5b2f9c'
         '340790b137f5fea2452a646d7c00be91'
         '7af99ffd1f0422cf4c37bcc88b79987e'
         '20258133f4319699fc37cab174f27830'
         'f60bb08fb582050c9e15f13f63cf283d'
         '431a0ce00ca6603746a5e26f29d0c590'
         '99d45043f950ba4e8f226bba6ae57583'
         '81d189bf88b93c51b42c0cab99c2f1e1'
         '10002f1e360f8de58eca38e2296936c5'
         '8ae91623f4198bab84bc867eda5d6c02'
         '32c49133fad64dc9431b52ba0adc1c72'
         '10f9c5f6a5ee09dcdbe42e9b9f78c98b'
         '1f33ec0eb6986f92322c8439b8484b46'
         '55ba386ce9d563fd7eda6a9d6dbf1c75')

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

	# makepkg copies local sources into srcdir using their base names.
	install -Dm644 "$srcdir/metadata.json" \
		"$pkgdir/usr/share/plasma/plasmoids/org.floitsch.shiftlayout.status/metadata.json"
	install -Dm644 "$srcdir/main.qml" \
		"$pkgdir/usr/share/plasma/plasmoids/org.floitsch.shiftlayout.status/contents/ui/main.qml"
	install -Dm644 "$srcdir/configGeneral.qml" \
		"$pkgdir/usr/share/plasma/plasmoids/org.floitsch.shiftlayout.status/contents/ui/configGeneral.qml"
	install -Dm644 "$srcdir/main.xml" \
		"$pkgdir/usr/share/plasma/plasmoids/org.floitsch.shiftlayout.status/contents/config/main.xml"
	install -Dm644 "$srcdir/config.qml" \
		"$pkgdir/usr/share/plasma/plasmoids/org.floitsch.shiftlayout.status/contents/config/config.qml"
}
