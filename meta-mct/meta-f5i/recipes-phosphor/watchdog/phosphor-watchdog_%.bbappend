FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Fix-watchdog-timeout-action-do-not-work.patch \
            file://0002-Implement-add-SEL-feature-for-watchdog-timeout.patch \
            file://0003-Add-post-code-SEL-for-FRB2-event.patch \
            file://0004-Add-watchdog2-timeout-flag-setting-feature.patch \
            file://0005-support-restart-cause-sel.patch \
            "

