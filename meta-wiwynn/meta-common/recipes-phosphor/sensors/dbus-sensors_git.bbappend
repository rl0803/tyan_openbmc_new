FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-to-configure-MaxValue-and-MinValue-in-ADC-sensor.patch \
            file://0002-Add-to-remove-PSU-PWM-sensor-and-to-fix-PSU-threshol.patch \
            file://0003-Add-to-support-VR-sensor-without-VR-driver.patch \
           "

SYSTEMD_SERVICE_${PN} += " xyz.openbmc_project.vrsensor.service"

DEPENDS += "obmc-libi2c \
            obmc-libmisc \
           "

RDEPENDS_${PN} += "obmc-libi2c \
                   obmc-libmisc \
                  "
