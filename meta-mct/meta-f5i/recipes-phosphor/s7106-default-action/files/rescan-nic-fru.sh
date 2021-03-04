#!/bin/bash

checkI2cDeviceExist=$(i2cdetect -y 17 |grep -i 51)
checkFileExist=$(ls /sys/bus/i2c/drivers/at24/17-0051)

if [[ ! -n $checkFileExist ]] && [[ -n $checkI2cDeviceExist ]]; then
    echo "Starting to rescan NIC FRU for intel NIC card"
    # Wait host power on
    sleep 10
    echo 17-0051 > /sys/bus/i2c/drivers/at24/bind
    systemctl restart  xyz.openbmc_project.FruDevice.service
fi

exit 0