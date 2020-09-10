FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://0001-Set-the-IPMB-transaction-retry-number-to-1.patch \
           "

SRC_URI_append = " file://ipmb-channels.json"

do_install_append(){
    install -m 0644 -D ${WORKDIR}/ipmb-channels.json \
                   ${D}/usr/share/ipmbbridge
}
