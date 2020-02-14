FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit native
inherit phosphor-settings-manager

SRC_URI += "file://defaults.yaml"

PROVIDES += "virtual/phosphor-settings-defaults"

S = "${WORKDIR}"

do_install() {
        DEST=${D}${settings_datadir}
        install -d ${DEST}
        install defaults.yaml ${DEST}
}
