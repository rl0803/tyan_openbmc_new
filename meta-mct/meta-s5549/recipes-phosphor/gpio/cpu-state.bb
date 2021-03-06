SUMMARY = "CPU related trigger application"
PR = "r1"

inherit obmc-phosphor-systemd
inherit obmc-phosphor-ipmiprovider-symlink
inherit pythonnative
inherit systemd
inherit obmc-phosphor-systemd
inherit cmake

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${MCTBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"

DEPENDS += "libpeci"

S = "${WORKDIR}/"
SRC_URI += " \
            file://CMakeLists.txt\
            file://LICENSE \
            file://caterrHandler.cpp \
            file://prochotThermtripHandler.cpp \
            file://xyz.openbmc_project.caterr.service \
            file://xyz.openbmc_project.prochot0.service \
            file://xyz.openbmc_project.prochot0_deassert.service \
            file://xyz.openbmc_project.thermtrip.service \
            "

DEPENDS += "sdbusplus sdbusplus-native"
DEPENDS += "phosphor-logging"
DEPENDS += "phosphor-dbus-interfaces phosphor-dbus-interfaces-native"
DEPENDS += "sdbus++-native"
DEPENDS += "libgpiod"
DEPENDS += "obmc-libmisc"

RDEPENDS_${PN} += "obmc-libmisc"

EXTRA_OECMAKE += "-DCATERR_ENABLE_POWER_FILTER=ON"
EXTRA_OECMAKE += "-DTHERMTRIP_ENABLE_POWER_FILTER=ON"
EXTRA_OECMAKE += "-DPROCHOT_ENABLE_POWER_FILTER=ON"


SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.caterr.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.prochot0.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.prochot0_deassert.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.thermtrip.service"

# linux-libc-headers guides this way to include custom uapi headers
CFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include"
CXXFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CXXFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include"
do_configure[depends] += "virtual/kernel:do_shared_workdir"


