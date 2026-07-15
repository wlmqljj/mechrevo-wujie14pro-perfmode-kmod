# Maintainer: wlmqljj <https://github.com/wlmqljj>
pkgname=mechrevo_wujie14pro_perfmode-dkms
_pkgname=mechrevo_wujie14pro_perfmode
pkgver=1.0
pkgrel=1
pkgdesc="Mechrevo WuJie14Pro performance mode DKMS kernel module"
arch=('x86_64')
url="https://github.com/wlmqljj/mechrevo-wujie14pro-perfmode-kmod"
license=('GPL')
depends=('dkms')
makedepends=()
source=("mechrevo_wujie14pro_perfmode.c"
        "Makefile"
        "dkms.conf"
        "mechrevo_wujie14pro_perfmode.conf")
md5sums=('SKIP' 'SKIP' 'SKIP' 'SKIP')

package() {
    local destdir="${pkgdir}/usr/src/${_pkgname}-${pkgver}"
    install -d "${destdir}"
    install -m644 mechrevo_wujie14pro_perfmode.c "${destdir}/"
    install -m644 Makefile "${destdir}/"
    install -m644 dkms.conf "${destdir}/"
    install -Dm644 mechrevo_wujie14pro_perfmode.conf "${pkgdir}/usr/lib/modules-load.d/mechrevo_wujie14pro_perfmode.conf"
}
