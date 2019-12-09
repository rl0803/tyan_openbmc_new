FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-to-configure-MaxValue-and-MinValue-in-ADC-sensor.patch \
            file://0002-Add-to-remove-PSU-PWM-sensor-and-to-fix-PSU-threshol.patch \
            file://0003-Add-to-support-VR-sensor-without-VR-driver.patch \
            file://0004-Support-Event-only-type-sensor-monitoring.patch \
            file://0005-Add-Invalid-Value-property-for-power-on-sensors.patch \
            file://0006-IPMB-type-sensors-to-support-the-invalid-value-property.patch \
            file://0007-Add-Button-event-only-sensor-for-button-pressed-event.patch \
            file://0008-Support-a-pgood-GPIO-read-func-for-power-status-checking.patch \
            file://0009-PSU-type-sensors-to-support-the-invalid-value-property.patch \
            file://0010-Add-End-of-Post-event-only-sensor.patch \
            file://0011-Support-cpu-peci-sensors-including-the-invalid-value-property.patch \
            file://0012-Add-to-support-NVMe-temperature-sensor.patch \
            file://0013-Add-to-support-NIC-temperature-sensor.patch \
            file://0014-FRU-state-sensor-for-power-cycle-SEL.patch \
            file://0015-Refer-to-the-pgood-property-for-powerStatusOn-check.patch \
            file://0016-Check-PSU-power-on-sensor-with-checkThresholdsPowerDelay.patch \
           "

SYSTEMD_SERVICE_${PN} += " xyz.openbmc_project.vrsensor.service \
                           xyz.openbmc_project.eventsensor.service \
                           xyz.openbmc_project.nvmesensor.service \
                           xyz.openbmc_project.nicsensor.service \
                         "

DEPENDS += "gpioplus \
            obmc-libi2c \
            obmc-libmisc \
            obmc-libgpio \
           "

RDEPENDS_${PN} += "obmc-libi2c \
                   obmc-libmisc \
                   obmc-libgpio \
                  "
