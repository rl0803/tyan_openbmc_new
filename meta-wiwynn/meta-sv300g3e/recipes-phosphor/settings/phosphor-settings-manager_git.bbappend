FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"
SRC_URI_append = "  file://time-owner.override.yml \
                    file://apply-time.override.yml \
                    file://power-restore-policy.override.yml \
                    file://sync-method.override.yml \
                    file://service-status.override.yml \
                 "
