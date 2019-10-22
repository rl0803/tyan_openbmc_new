# kcs channel 3 for BIOS
KCS_DEVICE = "ipmi-kcs3"

# kcs channel 2 for OS
KCS_DEVICE_OS = "ipmi-kcs2"
SYSTEMD_SERVICE_${PN} += " ${PN}@${KCS_DEVICE_OS}.service "
