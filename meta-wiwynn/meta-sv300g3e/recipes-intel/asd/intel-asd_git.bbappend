FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0002-Set-PLTRST_N-pin-type-to-PIN_NONE-to-avoid-changing-the-LPC-multi-func.patch \
            file://0003-Free-ASD-MSG-memory-after-running-asd_msg_free-function.patch \
            file://intel.asd.service \
           "

SYSTEMD_SERVICE_${PN} += "intel.asd.service"

do_install_append() {
    install -d ${D}/lib/systemd/system/
    install -m 0644 ${WORKDIR}/intel.asd.service ${D}/lib/systemd/system/
}
