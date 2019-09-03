FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-to-configure-MaxValue-and-MinValue-in-ADC-sensor.patch \
            file://0002-Add-to-remove-PSU-PWM-sensor-and-to-fix-PSU-threshol.patch \
           "
