FILESEXTRAPATHS_prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += "file://sv300g3e.cfg \
            file://aspeed-bmc-wiwynn-sv300g3e.dts;subdir=git/arch/${ARCH}/boot/dts \
            file://0001-at24.c-Disable-root-only.patch \
            file://0003-ftgmac100.c-Read-MAC-address-from-eeprom-sysfs.patch \
            file://0004-Add-I2C-IPMB-Support.patch \
            file://0005-Add-a-flash-layout-dtsi-for-a-64MB-mtd-device.patch \
           "

PACKAGE_ARCH = "${MACHINE_ARCH}"
