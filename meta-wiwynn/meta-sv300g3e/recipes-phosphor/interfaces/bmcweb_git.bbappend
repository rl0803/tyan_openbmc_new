FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-download-method-for-host-console-log.patch \
            file://0002-Enlarge-time-frame-for-TFTP-uploading-BMC-image.patch \
            file://0003-Add-http-download-method-for-ACD-crashdump.patch \
            file://0004-Add-Process-Name-to-BMC-Journal-Log-JSON-Response.patch \
            file://0005-Change-ACD-log-path-to-var-lib-crashdump.patch \
            "
# Enable CPU Log and Raw PECI support
EXTRA_OECMAKE_append += "-DBMCWEB_ENABLE_REDFISH_CPU_LOG=ON"
EXTRA_OECMAKE_append += "-DBMCWEB_ENABLE_REDFISH_RAW_PECI=ON"

# Enable Redfish BMC Journal support
EXTRA_OECMAKE_append += "-DBMCWEB_ENABLE_REDFISH_BMC_JOURNAL=ON"
EXTRA_OECMAKE_append += " -DBMCWEB_INSECURE_ENABLE_REDFISH_FW_TFTP_UPDATE=ON"
EXTRA_OECMAKE_append += " -DBMCWEB_HTTP_REQ_BODY_LIMIT_MB=128"
