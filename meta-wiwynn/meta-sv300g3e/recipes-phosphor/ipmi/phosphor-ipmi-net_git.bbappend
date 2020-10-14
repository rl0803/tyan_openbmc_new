FILESEXTRAPATHS_prepend_sv300g3e := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-netipmid-Skip-SOL-payload-activation-checking.patch \
            file://0002-Limit-the-host-console-buffer-size-to-1M.patch \
           "
