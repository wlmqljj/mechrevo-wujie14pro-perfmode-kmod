# Maintainer: wlmqljj
pkgname=mechrevo-wujie14pro-perfmode-dkms
pkgver=1.0
pkgrel=1
pkgdesc="Mechrevo WuJie14Pro performance mode DKMS kernel module"
arch=('x86_64')
url="https://github.com/wlmqljj/mechrevo-wujie14pro-perfmode-kmod"
license=('GPL')
depends=('dkms')
makedepends=()
source=("mechrevo-wujie14pro-perfmode.c"
        "Makefile"
        "dkms.conf"
        "mechrevo-wujie14pro-perfmode.conf")
md5sums=('SKIP'
         'SKIP'
         'SKIP'
         'SKIP')

package() {
    # 安装模块源代码到 /usr/src/
    install -d "${pkgdir}/usr/src/${pkgname}-${pkgver}"
    install -m644 mechrevo-wujie14pro-perfmode.c "${pkgdir}/usr/src/${pkgname}-${pkgver}/"
    install -m644 Makefile "${pkgdir}/usr/src/${pkgname}-${pkgver}/"
    install -m644 dkms.conf "${pkgdir}/usr/src/${pkgname}-${pkgver}/"

    # 安装模块配置文件到 /usr/lib/modules-load.d/
    install -Dm644 mechrevo-wujie14pro-perfmode.conf "${pkgdir}/usr/lib/modules-load.d/mechrevo-wujie14pro-perfmode.conf"
}
