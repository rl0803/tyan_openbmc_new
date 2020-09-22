LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${WIWYNNBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"
FILESEXTRAPATHS_append := "${THISDIR}/files:"

inherit systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}/"

SRC_URI = "file://init_once.sh \
           file://poweroff.sh \
           file://powersoft.sh \
           file://poweron.sh \
           file://poweron-delay.sh \
           file://powerreset.sh \
           file://powercyclelog.sh \
           file://fan-failure-poweroff.sh \
           file://host-gpio.service \
           file://host-poweroff.service \
           file://host-powersoft.service \
           file://host-poweron.service \
           file://host-poweron-delay.service \
           file://host-powerreset.service \
           file://host-powercyclelog.service \
           file://fan-failure-poweroff.service \
          "

DEPENDS = "systemd"
RDEPENDS_${PN} = "bash"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "host-gpio.service \
                         host-poweron.service \
                         host-poweron-delay.service \
                         host-poweroff.service \
                         host-powersoft.service \
                         host-powerreset.service \
                         host-powercyclelog.service \
                         fan-failure-poweroff.service \
                        "

do_install() {
    install -d ${D}/usr/sbin
    install -m 0755 ${S}init_once.sh ${D}/${sbindir}/
    install -m 0755 ${S}poweroff.sh ${D}/${sbindir}/
    install -m 0755 ${S}powersoft.sh ${D}/${sbindir}/
    install -m 0755 ${S}poweron.sh ${D}/${sbindir}/
    install -m 0755 ${S}poweron-delay.sh ${D}/${sbindir}/
    install -m 0755 ${S}powerreset.sh ${D}/${sbindir}/
    install -m 0755 ${S}powercyclelog.sh ${D}/${sbindir}/
    install -m 0755 ${S}fan-failure-poweroff.sh ${D}/${sbindir}/
}
