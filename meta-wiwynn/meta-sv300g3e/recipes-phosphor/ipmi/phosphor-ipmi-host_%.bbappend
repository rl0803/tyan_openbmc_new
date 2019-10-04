FILESEXTRAPATHS_prepend_sv300g3e := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-to-support-IPMI-Set-Power-Cycle-Interval-command.patch \
            file://0002-Add-to-support-IPMI-mc-warm-reset-command.patch \
           "

do_install_append(){
  install -d ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/include/ipmid/utils.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/include/ipmid/types.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/sensorhandler.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/selutility.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/storageaddsel.hpp ${D}${includedir}/phosphor-ipmi-host
}
