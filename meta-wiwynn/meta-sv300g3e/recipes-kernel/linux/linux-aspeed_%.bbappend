FILESEXTRAPATHS_prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += " file://sv300g3e.cfg "
SRC_URI += " file://aspeed-bmc-wiwynn-sv300g3e.dts;subdir=git/arch/${ARCH}/boot/dts "
SRC_URI += " file://0001-at24.c-Disable-root-only.patch "
# SRC_URI += " file://0002-nvmem-sysfs.c-Add-not-root-permission.patch "
SRC_URI += " file://0003-ftgmac100.c-Read-MAC-address-from-eeprom-sysfs.patch "

PACKAGE_ARCH = "${MACHINE_ARCH}"
