FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://sv300g3e-MB.json \
            file://Delta-DPS-500AB-PSU.json \
            file://fan-table.json \
            "

do_install_append(){
        install -d ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/*.json ${D}/usr/share/entity-manager/configurations
}
