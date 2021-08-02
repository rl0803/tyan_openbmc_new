FILESEXTRAPATHS_prepend_f5i := "${THISDIR}/${PN}:"

SRC_URI += "file://sdr.json \
            file://cs_privilege_levels.json \
           "

FILES_${PN} += "${datadir}/ipmi-providers/sdr.json"
FILES_${PN} += "${datadir}/ipmi-providers/cs_privilege_levels.json"


do_install_append() {
    install -m 0644 -D ${WORKDIR}/sdr.json \
    ${D}${datadir}/ipmi-providers/sdr.json

    install -m 0644 -D ${WORKDIR}/cs_privilege_levels.json \
    ${D}${datadir}/ipmi-providers/cs_privilege_levels.json
}




