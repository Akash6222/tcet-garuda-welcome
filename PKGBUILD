# Maintainer: dr460nf1r3 <dr460nf1r3 at tcetlinuxd dot org>
# Maintainer: librewish <librewish@tcetlinux.org>

pkgname=tcet-welcome
pkgver=1.2.1
pkgrel=1
pkgdesc="A Welcome app for tcet Linux"
arch=('any')
url="https://github.com/Akash6222"
license=('GPL3')
depends=('qt5-base' 'xdg-utils' 'garuda-libs>=1.3.0')
optdepends=( 'xorg-xrandr' 'xorg-xinput' 'garuda-assistant' 'garuda-network-assistant' 'qwikaccess' 'garuda-gamer' 'garuda-boot-options')

makedepends=('git')
groups=('tcet')


source=(${pkgname}::"git+${url}/${pkgname}")
sha256sums=('SKIP')

package() {
	install -dm755 ${pkgdir}${destname}
	cp -r ${srcdir}/${pkgname}${destname}/* ${pkgdir}${destname}
}
