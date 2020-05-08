FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"
SRC_URI_append = "  file://time-owner.override.yml \
                    file://apply-time.override.yml \
                 "
