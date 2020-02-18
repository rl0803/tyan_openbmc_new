FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Set-the-default-UART-route-setting.patch \
            file://0002-Set-default-fan-PWM-value.patch \
            file://0003-Configure-HICR5-register-value-to-default.patch \
           "
