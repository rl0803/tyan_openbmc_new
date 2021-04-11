SUMMARY = "System Restart Monitor"
DESCRIPTION = "Implement the Restart Monitor"
PR = "r1"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${WIWYNNBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"

inherit autotools pkgconfig
inherit obmc-phosphor-ipmiprovider-symlink
inherit pythonnative
inherit systemd
inherit obmc-phosphor-systemd
inherit obmc-phosphor-dbus-service


S = "${WORKDIR}"

SRC_URI = "file://bootstrap.sh \
           file://configure.ac \
           file://LICENSE \
           file://Makefile.am \
           file://utils.hpp \
           file://system-restart-monitor.cpp \
           file://system-restart-monitor.hpp \
           file://system-restart-monitor.service \
          "

DEPENDS += "autoconf-archive-native"
DEPENDS += "sdbusplus sdbusplus-native"
DEPENDS += "phosphor-logging"
DEPENDS += "phosphor-dbus-interfaces phosphor-dbus-interfaces-native"
DEPENDS += "sdbus++-native"
DEPENDS += "boost"
DEPENDS += "phosphor-dbus-interfaces"


RDEPENDS_${PN} += " \
        sdbusplus \
        phosphor-logging \
        phosphor-dbus-interfaces \
        "

SYSTEMD_SERVICE_${PN} = "system-restart-monitor.service"
