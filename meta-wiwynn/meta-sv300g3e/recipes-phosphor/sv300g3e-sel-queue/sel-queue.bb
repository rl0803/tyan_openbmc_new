SUMMARY = "SEL queue"
DESCRIPTION = "Store SEL in queue and send to sel logger"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${WIWYNNBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"

SRC_URI = "file://cmake \
           file://include \
           file://service \
           file://src \
           file://cmake-format.json \
           file://CMakeLists.txt \
           file://LICENSE \
          "

SYSTEMD_SERVICE_${PN} = "xyz.openbmc_project.sel-queue.service"

DEPENDS = "boost \
           nlohmann-json \
           sdbusplus \
          "
inherit cmake systemd
inherit obmc-phosphor-systemd

S = "${WORKDIR}"

EXTRA_OECMAKE = "-DYOCTO=1"

