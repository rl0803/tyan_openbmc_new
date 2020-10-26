FILESEXTRAPATHS_prepend_f5i := "${THISDIR}/${PN}:"

SRC_URI += "file://sdr.json"
FILES_${PN} += "${datadir}/ipmi-providers/sdr.json"

do_install_append() {
    install -m 0644 -D ${WORKDIR}/sdr.json \
    ${D}${datadir}/ipmi-providers/sdr.json
}




