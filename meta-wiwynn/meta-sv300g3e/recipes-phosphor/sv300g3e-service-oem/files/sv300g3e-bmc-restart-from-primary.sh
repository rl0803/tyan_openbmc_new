#!/bin/bash
# SV300G3-E BMC bmc restart from primary image

systemctl stop system-watchdog.service
systemctl stop xyz.openbmc_project.Logging.IPMI.service
systemctl stop xyz.openbmc_project.nicsensor.service
sleep 2

# Set /dev/mtdblock5 to read-only
CHECK=0
while [ ${CHECK} -lt 10 ]
do
    mount -o remount,ro /dev/mtdblock5
    if [ $? == 0 ]; then
        echo "${CHECK} - Set /dev/mtdblock5 to Read-Only OK!"
        break;
    fi
    echo "${CHECK} - Failed to set /dev/mtdblock5 to Read-Only!"
    (( CHECK=CHECK+1 ))
    sleep 1
done

# disable WDT2
WDT_DISABLE=0x10
WDT2_CTRL=0x1e78502c
devmem ${WDT2_CTRL} 32 ${WDT_DISABLE}

# write with 2 seconds to timeout
REBOOT_COUNT=0x1e8480
WDT2_RELOAD=0x1e785024
devmem ${WDT2_RELOAD} 32 ${REBOOT_COUNT}

# Reload value of WDT2
RESTART_CMD=0x4755
WDT2_RESTART=0x1e785028
devmem ${WDT2_RESTART} 32 ${RESTART_CMD}

# Read SCU30 to check boot at which BMC
SCU30=0x1e785030
VALWDT2=$(devmem ${SCU30} 32)
if [ $((${VALWDT2} & 0x02)) -eq 2 ]; then
    BOOT_SELECT1=0x92
    BOOT_SELECT2=0x93
else
    BOOT_SELECT1=0x12
    BOOT_SELECT2=0x13
fi

devmem ${WDT2_CTRL} 32 ${BOOT_SELECT1}
sleep 1
devmem ${WDT2_CTRL} 32 ${BOOT_SELECT2}
