FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"


SRC_URI += "file://f5i.cfg \
            file://0001-f5i-Initial-dts.patch \
            file://0002-S7106-Common-sourec-files.patch \
            file://0003-Implement-Heart-Beat-LED.patch \
            file://0004-Set-RTD_1_2_3-to-thermistor-mode.patch \
            file://0006-Modified-ibm-cffps-driver.patch \
            file://0007-Fix-fan-sensor-disappear-when-power-off-problem.patch \
            file://0008-s7106-increase-rofs-space.patch \
            file://0009-Register-all-of-dimm-sensors-when-initializing.patch \
            file://0010-change-aspeed-rpm-driver-to-falling-edge.patch \
            file://0011-irqchip-Add-Aspeed-SCU-Interrupt-Controller.patch \
            file://0012-Support-interrupt-handle-for-aspeed-bmc-misc.patch \
            file://0013-Add-debug-log-for-dimm-hwmon-sensor.patch \
            file://0014-misc-Add-clock-control-logic-into-Aspeed-LPC-SNOOP-d.patch \
            file://0015-media-aspeed-fix-clock-handling-logic.patch \
            file://0016-Update-PECI-driver-from-intel-kernel.patch \
            "

