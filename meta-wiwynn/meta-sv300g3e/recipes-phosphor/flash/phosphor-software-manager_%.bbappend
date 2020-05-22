FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-image-bmc-support-for-TFTP-updating-BMC.patch \
            file://0002-Version-check-disable-for-OOB-BMC-update.patch \
            file://0003-Add-BIOS-update-flow.patch \
           "
PACKAGECONFIG_append = " flash_bios"
