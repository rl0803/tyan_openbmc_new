FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

EXTRA_OECMAKE += "-DSEL_LOGGER_MONITOR_THRESHOLD_EVENTS=ON"

SRC_URI += "file://0001-Reset-the-SEL-ID-to-1-after-clearing-the-SEL.patch \
            file://0002-Set-BMC-status-LED-while-critical-events-occur.patch \
            file://0003-Synchronize-time-with-ME.patch \
            file://0004-Get-the-asserted-sensor-value-from-AssertValue-property.patch \
            file://0005-Add-a-new-dbus-connection-for-internal-method-call.patch \
            file://0006-Add-judgment-to-prevent-assertCount-from-being-negative.patch \
            file://0007-Disable-time-sync-with-ME-function.patch \
            file://0008-Set-sel-logger-to-wait-for-METimeSync-service.patch \
            file://0009-Support-battery-low-SEL-record.patch \
            file://0010-Modify-sel-logger-get-scale-raw-data-from-double.patch \
           "
