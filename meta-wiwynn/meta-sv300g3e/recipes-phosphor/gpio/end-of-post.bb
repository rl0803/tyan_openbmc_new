LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${WIWYNNBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"


SUMMARY = "End of Post Event Log Application"
PR = "r1"

inherit obmc-phosphor-systemd

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://end-of-post-event.sh"

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 ${WORKDIR}/end-of-post-event.sh \
            ${D}${sbindir}/end-of-post-event.sh
}

SYSTEMD_ENVIRONMENT_FILE_${PN} +="obmc/gpio/end_of_post"

END_OF_POST_SERVICE = "end_of_post"

TMPL = "phosphor-gpio-monitor@.service"
INSTFMT = "phosphor-gpio-monitor@{0}.service"
TGT = "${SYSTEMD_DEFAULT_TARGET}"
FMT = "../${TMPL}:${TGT}.requires/${INSTFMT}"

SYSTEMD_SERVICE_${PN} += "end-of-post-event.service"
SYSTEMD_LINK_${PN} += "${@compose_list(d, 'FMT', 'END_OF_POST_SERVICE')}"
