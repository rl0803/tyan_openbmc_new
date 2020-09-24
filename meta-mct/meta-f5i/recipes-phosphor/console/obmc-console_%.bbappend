FILESEXTRAPATHS_prepend_f5i := "${THISDIR}/${PN}:"
OBMC_CONSOLE_HOST_TTY = "ttyS2"

SRC_URI += "file://obmc-console-ssh@.service \
            file://0001-Support-start-preparing-and-stop-commnad-configurati.patch \
            "

