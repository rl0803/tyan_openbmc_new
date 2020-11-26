FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-to-configure-MaxValue-and-MinValue-in-ADC-sensor.patch \
            file://0003-Add-to-support-VR-sensor-without-VR-driver.patch \
            file://0004-Support-Event-only-type-sensor-monitoring.patch \
            file://0005-Add-Invalid-Value-property-for-power-on-sensors.patch \
            file://0006-IPMB-type-sensors-to-support-the-invalid-value-property.patch \
            file://0009-PSU-type-sensors-to-support-the-invalid-value-property.patch \
            file://0011-Support-cpu-peci-sensors-including-the-invalid-value-property.patch \
            file://0012-Add-to-support-NVMe-temperature-sensor.patch \
            file://0013-Add-to-support-NIC-temperature-sensor.patch \
            file://0015-Refer-to-the-pgood-property-for-powerStatusOn-check.patch \
            file://0019-Tach-type-Fan-sensors-to-support-the-invalid-value-property.patch \
            file://0022-Set-non-blocking-sync-read-to-PECI-type-sensors.patch \
            file://0023-Set-the-sensor-value-of-absent-DIMM-as-invalid.patch \
            file://0024-Add-to-support-CATERR-event-handler.patch \
            file://0025-Remove-Tjmax-from-hiddenProps-for-sensor-creation.patch \
            file://0026-Set-the-tach-sensor-value-to-0-when-failing-to-read.patch \
            file://0027-Add-to-support-Fan-power-sensor.patch \
            file://0028-Change-to-steady_timer-used-in-ThresholdTimer.patch \
            file://0029-Change-to-steady_timer-used-in-ADC-sensor.patch \
            file://0030-Change-to-steady_timer-used-in-CPU-sensor.patch \
            file://0031-Change-to-steady_timer-used-in-FanExt-sensor.patch \
            file://0032-Change-to-steady_timer-used-in-HwmonTemp-sensor.patch \
            file://0033-Change-to-steady_timer-used-in-Ipmb-sensor.patch \
            file://0034-Change-to-steady_timer-used-in-NIC-sensor.patch \
            file://0035-Change-to-steady_timer-used-in-NVME-sensor.patch \
            file://0036-Change-to-steady_timer-used-in-FanTach-sensor.patch \
            file://0037-Change-to-steady_timer-used-in-VR-sensor.patch \
            file://0038-Change-to-steady_timer-used-in-PSU-sensor.patch \
            file://0039-Support-CATERR-LED-feature.patch \
            file://0040-Fix-sensor-hysteresis-issue.patch \
            file://0041-Add-an-event-only-sensor-for-SEL-entry-event.patch \
            file://0042-Stop-the-threshold-check-delay-timer-when-the-event-is-deasserted.patch \
            file://0043-Implement-BMC-Update-SEL.patch \
            file://0044-Set-event-only-sensor-service-to-wait-for-dbus-sel-logging-object.patch \
            file://0045-PSU-sensor-via-ME-proxy-command.patch \
            file://0046-Support-NIC-card-temperature-over-MCTP.patch \
            file://0047-Clear-the-PSU-fault-for-once-after-creating-the-PSU-status-object.patch \
            file://0048-Support-ADC-polling-time-configuration.patch \
            file://0049-Add-WDT2-and-Fast-Prochot-event-only-sensors.patch \
            file://0050-Add-Fan_Failure-event-only-sensor.patch \
            file://0051-Support-2-fan-failure-event-handler.patch \
            file://0052-Use-different-IIDs-in-NIC-temperature-via-MCTP-method.patch \
            file://0053-Add-Fan-Failure-SEL-entry-to-the-normal-sel-add-queue.patch \
            file://0054-Add-PSU-STATUS-entry-to-the-500ms-delay-sel-add-queue.patch \
            file://0055-Add-boundary-retry-for-ADC-sensor.patch \
            file://0056-Add-Battery-event-only-sensor.patch \
            file://0057-Refer-to-the-AC-lost-flag-for-AC-lost-SEL-recording.patch \
            "

SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.vrsensor.service \
                          xyz.openbmc_project.eventsensor.service \
                          xyz.openbmc_project.nvmesensor.service \
                          xyz.openbmc_project.nicsensor.service \
                          xyz.openbmc_project.fanextsensor.service \
                          xyz.openbmc_project.psuproxymesensor.service \
                         "

DEPENDS += "obmc-libi2c \
            obmc-libmisc \
           "

RDEPENDS_${PN} += "obmc-libi2c \
                   obmc-libmisc \
                  "
# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
