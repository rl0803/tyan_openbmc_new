FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-s7106-Support-baseboard-fru-reading.patch \
            file://s7106-Baseboard.json \
            file://b7106-Baseboard.json \
            file://Chicony-R550-PSU.json \
            file://Delta-DPS-500AB-PSU.json \
            file://Delta-DPS-550AB-PSU.json \
            file://Delta-DPS-650AB-PSU.json \
            file://Intel-V710-NIC.json \
            file://fru.json \
            file://blacklist.json \
            file://0004-fix-fsc-to-settable-interface.patch \
            file://0005-remove-NVME-P4000.json.patch \
            file://0006-Set-the-EntityManager-service-booting-after-FruDevic.patch \
            file://0007-Fix-fru-disappear-after-power-off.patch \
            file://0008-Add-FRU-device-read-only-property.patch \
            "

do_install_append(){
	install -d ${D}/usr/share/entity-manager/configurations
	install -m 0444 ${WORKDIR}/*.json ${D}/usr/share/entity-manager/configurations
	install -m 0444 ${WORKDIR}/blacklist.json ${D}/usr/share/entity-manager
}
