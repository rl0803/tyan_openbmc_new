FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://0001-Add-fan-control-linear.patch \
             file://0002-Modify-the-fan-control-pid-process.patch \
             file://0003-Add-type-linear-and-debug-manual-mode.patch \
           "

inherit obmc-phosphor-systemd
SYSTEMD_SERVICE_${PN} = "phosphor-pid-control.service"
EXTRA_OECONF = "--enable-configure-dbus=no"
