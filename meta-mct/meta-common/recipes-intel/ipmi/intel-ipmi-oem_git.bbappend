FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-s7106-disable-Intel-OEM-command.patch \
            file://0003-add-SDR-type-3-support.patch \
            file://0009-support-AddSelEntry-command.patch \
            file://0011-Change-ipmi_sel-location-to-persistent-folder.patch \
            file://0014-implement-defined-sdr-table.patch \
            file://0016-Support-the-reading-unavailable-bit.patch \
            file://0017-fix-negative-value-issue.patch \
            file://0018-support-leaky-bucket.patch \
            file://0020-support-fru-sync-to-smbios.patch \
            file://0021-workaroud-for-ipmitool-fru-edit-issue.patch \
            file://0022-IPMB-support-for-NM-commands.patch \
            file://0023-f5i-ignore-return-check-when-updating-hash-list.patch \
            file://0024-Support-watchdog2-sensor-type.patch \
            file://0025-sol-pettern-sensor-support.patch \
            file://0026-Implement-the-event-log-for-clear-SEL-command.patch \
            file://0027-Fix-the-sdr-record-counter-error.patch \
            file://0028-Implement-the-SEL-log-feature-for-platform-event-com.patch \
            file://0029-Support-FRU-configuration-in-JSON-format.patch \
            file://0030-Support-system-event-sensor-type.patch \
            file://0031-Add-dbus-match-function-for-rescan-FRU-cache.patch \
            file://0032-Add-dbus-signal-function-for-clear-SEL-feature.patch \
            "

