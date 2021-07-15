FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0002-Add-interface-for-caterr-logging.patch \
            file://0003-Add-SOL-PATTERN-Interface-for-SOL-Processing.patch \
            file://0004-Add-interface-for-AC-restore-always-on-policy.patch \
            file://0005-Add-interface-for-setting-specified-service-status.patch \
            file://0007-Add-interface-for-DCMI-power.patch \
            file://0009-Add-the-version-ID-parameter-to-software-version-int.patch \
            file://0010-Add-interface-for-setting-specified-host-status.patch \
            file://0011-Update-dump-entry-and-add-Notify-method.patch \
            file://0012-InitiateOffload-method-for-the-dump-entry.patch \
            file://0013-Add-property-for-restart-cause-in-host-service.patch \
            file://0014-Add-property-for-AMT-monitor-sattus-in-host-status-i.patch \
            file://0015-Update-watchdog-default-timeout-time-to-300-second.patch \
            "

