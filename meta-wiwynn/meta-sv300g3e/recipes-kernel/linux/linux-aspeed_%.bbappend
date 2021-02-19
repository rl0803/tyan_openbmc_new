FILESEXTRAPATHS_prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += "file://sv300g3e.cfg \
            file://aspeed-bmc-wiwynn-sv300g3e.dts;subdir=git/arch/${ARCH}/boot/dts \
            file://0001-at24.c-Disable-root-only.patch \
            file://0003-ftgmac100.c-Read-MAC-address-from-eeprom-sysfs.patch \
            file://0004-Add-I2C-IPMB-Support.patch \
            file://0005-Add-a-flash-layout-dtsi-for-a-64MB-mtd-device.patch \
            file://0006-Change-default-fan-PWM-to-80.patch \
            file://0007-Modify-WDT2-timeout-from-30s-to-65s.patch \
            file://0008-Export-all-DIMMs-during-initialization.patch \
            file://0009-Support-CPU-margin-reading.patch \
            file://0010-Add-AST2500-JTAG-driver-support.patch \
            file://0011-Modify-PTCR-register-for-setting-desired-minimal-RPM.patch \
            file://0012-Add-clear-buffer-in-slave-mqueue-driver.patch \
            file://0013-Clear-the-i2c-slave-state-when-a-bmc-i2c-master-cmd-times-out.patch \
            file://0014-write-SRAM-panic-words-to-record-kernel-panic.patch \
            file://0015-misc-Add-clock-control-logic-into-Aspeed-LPC-SNOOP-d.patch \
            file://0016-media-aspeed-fix-clock-handling-logic.patch \
            file://0017-Remove-Unregister-SMC-Partitions.patch \
            file://0018-Not-to-start-BMC-watchdog-during-wdt-device-probe.patch \
            file://0019-Change-ADC-engine-control-setting-and-reading-function.patch \
            file://0020-Fix-incorrect-RJ45-LED-status.patch \
           "

PACKAGE_ARCH = "${MACHINE_ARCH}"
