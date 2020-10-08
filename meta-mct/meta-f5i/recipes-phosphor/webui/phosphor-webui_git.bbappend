FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRCREV = "0c4aceb775cbb461d10e03888f0ab90e5a619dc7"

SRC_URI += "file://0002-Implement-the-feature-for-setting-web-status-to-enab.patch \
            file://0003-Fix-the-page-for-sensors-reading-feature.patch \
            file://0004-Fix-the-page-for-manage-power-usage.patch \
            file://0005-Disable-unfinished-feature-temporarily.patch \
"

