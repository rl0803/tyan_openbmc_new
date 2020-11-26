FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

pkg_postinst_ontarget_${PN}() {
    systemctl stop dropbear.socket
    systemctl disable dropbear.socket
}
