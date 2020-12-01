FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# SRCREV = "c0dcf0d3f0a0865ccc9ff3ada2d3e70918377401"

SRC_URI += "file://sv300g3e-MB.json \
            file://sv300g3e-BIOS.json \
            file://Delta-DPS-500AB-PSU.json \
            file://Delta-DPS-550AB-PSU.json \
            file://Chicony-R550-PSU.json \
            file://blacklist.json \
            file://0002-Set-the-boot-order-of-EntityManager-after-FruDevice.patch \
            file://0003-Get-PSU-FRU-via-ME-proxy.patch \
            file://0004-Add-FRU-device-read-only-property.patch \
            "

do_install_append(){
        install -d ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/sv300g3e-MB.json ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/sv300g3e-BIOS.json ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/Delta-DPS-500AB-PSU.json ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/Delta-DPS-550AB-PSU.json ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/Chicony-R550-PSU.json ${D}/usr/share/entity-manager/configurations

        install -m 0444 ${WORKDIR}/blacklist.json ${D}/usr/share/entity-manager
}
