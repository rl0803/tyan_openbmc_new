FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SYSTEMD_ENVIRONMENT_FILE_${PN}-host += "obmc/phosphor-reboot-host/reboot.conf"

SRC_URI += "file://0001-Determine-the-host-initial-state-with-pgood.patch \
            file://0002-Support-random-power-on-feature-to-always-on-policy.patch \
            file://0003-Support-power-cycle-interval.patch \
            file://0004-Run-power-policy-service-for-AC-lost-event-only.patch \
            file://0005-Add-power-on-host-after-multi-user-target.patch \
           "
