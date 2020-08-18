FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-remove-the-sesnor-data-scaling.patch \
            file://0002-f5i-fan-speed-control.patch \
            file://0003-fix-Tjmax-change-don-t-take-effect-issue.patch \
            file://0004-Fix-the-pid-coredump-issue.patch \
            "

