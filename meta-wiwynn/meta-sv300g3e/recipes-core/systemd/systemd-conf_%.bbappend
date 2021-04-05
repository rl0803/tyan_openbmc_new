FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
            file://disable-forward-to-syslog.conf \
           "

do_install_append() {
        install -m 644 -D ${WORKDIR}/disable-forward-to-syslog.conf ${D}${systemd_unitdir}/journald.conf.d/disable-forward-to-syslog.conf
}

FILES_${PN} += "${systemd_unitdir}/journald.conf.d/disable-forward-to-syslog.conf"
