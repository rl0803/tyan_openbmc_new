FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-property-for-power-on-dealy-policy.patch \
            file://0002-Add-OEM-interface-to-enable-services.patch \
            file://0003-Add-property-for-restart-cause-in-host-service.patch \
           "
