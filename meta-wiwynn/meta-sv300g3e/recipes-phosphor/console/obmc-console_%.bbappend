FILESEXTRAPATHS_prepend_sv300g3e := "${THISDIR}/${PN}:"
OBMC_CONSOLE_HOST_TTY = "ttyS2"

SRC_URI += "file://0001-Add-timestamps-to-SOL-buffer.patch"
