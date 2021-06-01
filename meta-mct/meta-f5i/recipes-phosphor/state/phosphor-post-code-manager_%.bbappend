FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Implement-the-clear-bios-post-code-feature-for-host-.patch \
            file://0002-Print-out-current-boot-cycle-index-to-journal-log-fo.patch \
           "

