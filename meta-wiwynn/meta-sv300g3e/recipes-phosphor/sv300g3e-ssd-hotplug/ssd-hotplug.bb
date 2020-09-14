SUMMARY = "SSD hot-plug event handler"
DESCRIPTION = "SSD hot-plug event SEL entry"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "file://cmake \
           file://include \
           file://service \
           file://src \
           file://cmake-format.json \
           file://CMakeLists.txt \
           file://LICENSE \
          "

SYSTEMD_SERVICE_${PN} = "xyz.openbmc_project.ssd-hotplug-event.service"

inherit cmake systemd
inherit obmc-phosphor-systemd

DEPENDS += "boost nlohmann-json sdbusplus libgpiod obmc-libi2c"
RDEPENDS_${PN} += "libsystemd sdbusplus libgpiod obmc-libi2c"


S = "${WORKDIR}"

EXTRA_OECMAKE = "-DYOCTO=1"

do_install_append() {
    install -d ${D}/etc
}
