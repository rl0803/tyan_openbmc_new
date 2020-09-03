inherit obmc-phosphor-dbus-service
inherit obmc-phosphor-systemd

FILESEXTRAPATHS_append := "${THISDIR}/files:"
SUMMARY = "CPU Crashdump"
DESCRIPTION = "CPU utilities for dumping CPU Crashdump and registers over PECI"

DEPENDS = "boost cjson sdbusplus safec gtest libpeci"

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=43c09494f6b77f344027eea0a1c22830"
SRCREV = "wht-0.9"

S = "${WORKDIR}/"
SRC_URI = "file://crashdump-0.9.tgz \
           file://LICENSE  \
           file://crashdump_input_clx.json \
          "

INSANE_SKIP_${PN} += "already-stripped"
INSANE_SKIP_${PN} += "ldflags"

SYSTEMD_SERVICE_${PN} += "com.intel.crashdump.service"
DBUS_SERVICE_${PN} += "com.intel.crashdump.service"
FILES_${PN} += "/usr/share/*"

do_install() {
    install -d ${D}/usr/bin
    install -m 0755 ${S}crashdump ${D}/usr/bin/
    install -d ${D}/usr/share/crashdump/input
    install -m 0644 ${S}crashdump_input_clx.json ${D}/usr/share/crashdump/input/crashdump_input_clx.json
}
