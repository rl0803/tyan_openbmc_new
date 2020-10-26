SUMMARY = "Twitter OEM IPMI commands"
DESCRIPTION = "Twitter OEM Commands"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${MCTBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"

S = "${WORKDIR}/"

SRC_URI = "file://libttoemcmds.so.0.1.0.tgz \
           file://LICENSE \
          "

DEPENDS += "boost phosphor-ipmi-host phosphor-logging systemd"
DEPENDS += "libgpiod libpeci"

inherit obmc-phosphor-ipmiprovider-symlink

INSANE_SKIP_${PN} += "already-stripped"
HOSTIPMI_PROVIDER_LIBRARY += "libttoemcmds.so"

FILES_${PN}_append = " ${libdir}/ipmid-providers/lib*${SOLIBS}"
FILES_${PN}_append = " ${libdir}/host-ipmid/lib*${SOLIBS}"
FILES_${PN}_append = " ${libdir}/net-ipmid/lib*${SOLIBS}"
FILES_${PN}-dev_append = " ${libdir}/ipmid-providers/lib*${SOLIBSDEV} ${libdir}/ipmid-providers/*.la"

do_install() {
    install -d ${D}/usr/lib/ipmid-providers
    cp -P ${S}libttoemcmds.so.0* ${D}/usr/lib/ipmid-providers
}
