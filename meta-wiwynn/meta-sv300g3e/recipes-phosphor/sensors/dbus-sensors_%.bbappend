FILESEXTRAPATHS_append := "${THISDIR}/files:"

# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
