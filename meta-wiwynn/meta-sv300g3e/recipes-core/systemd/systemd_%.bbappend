FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://0001-Disable-NTP-time-sync-by-default.patch \
           "

pkg_postinst_ontarget_${PN}() {
    systemctl stop dropbear.socket
    systemctl disable dropbear.socket
}
